#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/dir.h>

#include <map>
#include <boost/serialization/map.hpp>

#include <stack>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/stack.hpp>

#include <sstream>
#include <fstream>

#include "vms.hpp"
#include "utils.h"
#include "archive.hpp"
#include "commit.hpp"
#include "blob.hpp"
#include "access.hpp"


using namespace std;

const string DELETED_FILE = "DELETED";

int move_from_cache_to_objects(const string& hash) {
    ostringstream cache_path;
    cache_path << ".vms/cache/" << hash;

    ostringstream objects_path;
    objects_path << ".vms/objects/" << hash;
    
    int ret = move_file(cache_path.str().c_str(), objects_path.str().c_str());

    if (ret != 0) {
        cerr << "ERROR: Unable to move staged changes from cache to objects directory" << endl;
        return -1;
    }
        // Change permissions of generated file.
    ret = chmod(objects_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Failed to change permissions of file." << objects_path.str() << endl;
    }

    return 0;
}


int vms_init() {

    // Initialize directories
    int ret = make_dir(".vms");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/objects");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/branches");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/cache");
    if (ret != 0) {
        return -1;
    }

    // Initialize files
    map<string, string> index;
    save< map<string, string> >(index, ".vms/index");

    stack<string> log;
    save< stack<string> >(log, ".vms/log");

    // Initialize initial commit
    Commit sentinal;
    string sentinal_hash = sentinal.hash();

    create_and_write_file(".vms/HEAD", "master", 0644);
    create_and_write_file(".vms/branches/master", sentinal_hash.c_str(), 0644);

    ostringstream obj_path;
    obj_path << ".vms/objects/" << sentinal_hash;
    save<Commit>(sentinal, obj_path.str());

    return 0;
}

int vms_stage(char* filepath) {
    // TODO: add support for staging change to deleted file 
    // (e.g. if previously tracked file but file now is deleted)

    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // if file was previously being tracked but is now deleted
    if(is_tracked_file(filepath) && !is_valid_file(filepath)) {
        // Puts filepath into index map with special string, save the updated index, and return
        index[filepath] = DELETED_FILE;
        save< map<string, string> >(index, ".vms/index");
        return 0;
        
    } else if (is_staged_file(filepath) && !is_valid_file(filepath)) { // if file was previously staged but is now deleted
        // Remove the file from index, save the updated index, and return
        index.erase(string(filepath));
        save< map<string, string> >(index, ".vms/index");
        return 0;

    }


    // Blob the file's contents and get its hash
    ifstream ifilestream(filepath);
    if (!ifilestream.is_open()) {
        cout << "ERROR: Unable to open file" << filepath << endl;
    }
    Blob file_blob(ifilestream);

    string file_hash = file_blob.hash();

    // Puts filepath and hash into the index map and save the updated index
    index[filepath] = file_hash;
    save< map<string, string> >(index, ".vms/index");

    // Save blob in a temporary folder. This serves as cache.
    ostringstream obj_cache_path;
    obj_cache_path << ".vms/cache/" << file_hash;
    save<Blob>(file_blob, obj_cache_path.str());

    return 0;
}

int vms_unstage(char* filepath) {
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // remove entry from the index map and save the updated index
    index.erase(string(filepath));
    save< map<string, string> >(index, ".vms/index");

    // Note: decide to not remove cache because unnecessary: will clear cache after commits
    
    return 0;
}

int vms_commit(char* msg) {
    // TODO: Fix and be more rigorous with error handling and propagation
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    
    // Check if any tracked changes to commit, if not print and return, otherwise continue
    if (index.empty()) {
        cout << "No changes staged to commit" << endl;
        return -1;
    }

    // Create new commit, update its internal map, and move files from cache to objects directory
    Commit commit(msg);
    cout << "commit before adding new elements" << endl;
    commit.print();

    map<string,string>::iterator it;
    for (it=index.begin(); it!=index.end(); ++it) {
        // if staged is file mapped to DELETED_FILE string, remove it from commit tree
        if (it->second == DELETED_FILE) {
            commit.remove_from_map(it->first);
        } else { // puts entry to commit map and move files from cache to objects directory
            commit.put_to_map(it->first, it->second);
            move_from_cache_to_objects(it->second);
        }
    }
    cout << "commit after adding new elements" << endl;
    commit.print();


    // Iterate through cache directory, clearing it
    DIR *dirptr = opendir(".vms/cache");
    struct dirent *entry = readdir(dirptr);
    
    char cache_file_path[BUFSIZ];
    
    while (entry != NULL) {

        sprintf(cache_file_path, "%s/%s", ".vms/cache", entry->d_name);

        if (is_valid_file(cache_file_path)) {
            remove_file(cache_file_path);
        }

        entry = readdir(dirptr);

    }

    // Clear index and save it
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    // Change position of branch pointed to by HEAD
    string commit_hash = commit.hash();
    
    string branch_fpath = get_branch_path();
    
    create_and_write_file(branch_fpath.c_str(), commit_hash.c_str(), 0644);

    //Push formatted commit string to log and save
    stack<string> log;
    restore< stack<string> >(log, ".vms/log");

    log.push(commit.log_string());
    save< stack<string> >(log, ".vms/log");

    // Serialize and save your commit.
    ostringstream obj_path;
    obj_path << ".vms/objects/" << commit_hash;
    save<Commit>(commit, obj_path.str());

    // Change permissions of generated file.
    int ret = chmod(obj_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Failed to change permissions of file." << obj_path.str() << endl;
    }

    return 0;
}

int vms_log() {
    // TODO: Add option to print log of only top n entries
    // Load log
    stack<string> log;
    restore< stack<string> >(log, ".vms/log");
    while (!log.empty()) {
        cout << "===" << endl;
        cout << log.top() << endl;
        log.pop();
    }

    return 0;
}

int vms_status() {
    // List what branches currently exist and mark current branch
    string current_branch = get_branch();

    DIR *dirptr = opendir(".vms/branches");
    struct dirent *entry = readdir(dirptr);

    cout << "=== Branches ===" << endl;

    while (entry != NULL) {
        if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
            if (strcmp(current_branch.c_str(), entry->d_name) == 0) {
                cout << "*";
            }

            cout << entry->d_name << endl;

        }
        
        entry = readdir(dirptr);

    }

    cout << endl;


    // List all files currently staged. (and list type of modification: modified, deleted)
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    map<string,string>::iterator it;
    cout << "=== Staged Files ===" << endl;

    for (it = index.begin(); it != index.end(); ++it) {
        // if new (not currently tracked)
        if (!is_tracked_file(it->first.c_str())) {

            cout << "new file:    " << it->first << endl;

        } else if (it->second == DELETED_FILE) {    // if deleted (mapped to delete)

            cout << "deleted:     " << it->first << endl;

        } else if (is_modified_tracked_file(it->first.c_str())) { // if modified
            cout << "modified:    " << it->first << endl;
        } 

        // Note: the order of these checks is important
        
    }
    cout << endl;



    // List all files that have been staged and have been modified since staging (and the type of modification)
    cout << "=== Unstaged Changes ===" << endl;

    // TODO: Improve performance of this loop.
    // TODO: Complicated logic to achieve desired behavior: simplify control logic if you can, to make things more clear.
    for (it = index.begin(); it != index.end(); ++it) {

        if (it->second != DELETED_FILE) {
            if (!is_valid_file(it->first.c_str())) {    // if previously staged file has been deleted, list it as deleted
                cout << "deleted:     " << it->first << endl;
            } else if (!file_hash_equal_to_working_copy(it->first, it->second)) {    // if tracked file has been modified, list it as modified
                cout << "modified:    " << it->first << endl;
            }

        } else { // File staged as deleted
            if (is_valid_file(it->first.c_str())) { // if file is no longer deleted and can be staged to be added again, should notify user.. by definition, it is already tracked, so maybe check if it has changes. If so, then list it as modified... if it doesn't... then problem because can't stage if no changes to stage. Easy way around it is to remove the check before staging, but bad for performance because need to do check in status loop. But this loop is fairly small, supposedly. Okay.
                    cout << "modified:    " << it->first << endl;
            }
        }
    }
    // list all tracked files that have not been staged and have been modified (and the type of modification)
    // Load parent commit
    Commit parent_commit;
    string parent_hash = get_parent_ref();
    ostringstream parent_fpath;
    parent_fpath << ".vms/objects/" << parent_hash;

    restore<Commit>(parent_commit, parent_fpath.str());

    // Get reference to map of parent commit
    map<string, string>& parent_map_ref = parent_commit.get_map();

    for (it = parent_map_ref.begin(); it != parent_map_ref.end(); ++it) {
        if (!is_staged_file(it->first.c_str())) {
            if (!is_valid_file(it->first.c_str())) { // unstaged tracked file has been deleted from working directory

                cout << "deleted:     " << it->first << endl;
            
            } else if (is_modified_tracked_file(it->first.c_str())) {
            
                cout << "modified:    " << it->first << endl;
            
            }
        }
    }



    // list all deleted files that used to be tracked
    // list all untracked files in this directory

    // std::map<std::string,std::string>::iterator it;
    // std::cout << "Map contains:" << std::endl;
    // for (it=index.begin(); it!=index.end(); ++it) {
    //     std::cout << it->first << " => " << it->second << std::endl;
    // }

    return 0;
}
