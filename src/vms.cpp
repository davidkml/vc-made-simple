#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/dir.h>

#include <map>
#include <boost/serialization/map.hpp>

#include <set>
#include <list>
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

    // Create subdirectory if it does not already exist and move
    ostringstream objects_path;

    string hash_prefix;
    string hash_suffix;
    split_prefix_suffix(hash, hash_prefix, hash_suffix, PREFIX_LENGTH);

    objects_path << ".vms/objects/" << hash_prefix;
    make_dir(objects_path.str().c_str()); // May need some error handling code
    
    objects_path << "/" << hash_suffix;
    int ret = move_file(cache_path.str().c_str(), objects_path.str().c_str());

    if (ret != 0) {
        cerr << "Error occurred: unable to move staged changes from cache to objects directory" << endl;
        return -1;
    }
    // Change permissions of generated file.
    ret = chmod(objects_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Error occurred: unable to change permissions of blob file." << objects_path.str() << endl;
        return -1;
    }

    return 0;
}

/** Helper method for restoring a commit from a shortened commit id
 * Utilization assumes prior validation that shortened id is valid, meaning:
 * - it is longer than the defined PREFIX_LENGTH
 * - its prefix matches a subdirectory in the .vms/objects directory
 * - its suffix matches exactly one file in the .vms/objects/<prefix> subdirectory **/
int restore_commit_from_shortened_id(const char* commit_id, Commit& commit) {
    string id_prefix;
    string id_suffix;
    split_prefix_suffix(string(commit_id), id_prefix, id_suffix, PREFIX_LENGTH);
    
    ostringstream full_id;
    full_id << id_prefix;

    ostringstream obj_path;
    obj_path << ".vms/objects/" << id_prefix;

    const char* obj_subdir = obj_path.str().c_str();

    if (is_valid_dir(obj_subdir)) {
    
        DIR *dirptr = opendir(obj_subdir);
        struct dirent *entry = readdir(dirptr);

        while (entry != NULL) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (strncmp(id_suffix.c_str(), entry->d_name, id_suffix.length()) == 0) {

                    obj_path << "/" << entry->d_name;
                    full_id << entry->d_name;

                }
            }
            entry = readdir(dirptr);
        }
    }
    restore<Commit>(commit, obj_path.str());

    // verify no tampering or corruption of restored object
    if (commit.hash() != full_id.str()) {
        cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << endl;
        return -1;
    }

    return 0;
}

int restore_blob_from_full_id(const string& blob_id, Blob& blob) {
    string blob_id_prefix;
    string blob_id_suffix;
    split_prefix_suffix(blob_id, blob_id_prefix, blob_id_suffix, PREFIX_LENGTH);

    ostringstream blob_path;
    blob_path << ".vms/objects/" << blob_id_prefix << "/" << blob_id_suffix;

    restore<Blob>(blob, blob_path.str());

    // verify no tampering or corruption of restored object
    if (blob.hash() != blob_id) {
        cerr << "Fatal error has occurred in retrieval of file contents: uuid mismatch. Archived object may have been corrupted. Exiting..." << endl;
        return -1;
    }

    return 0;
}

/** Creates directory path to file if it doesn't already exist and returns position of the end slash, or 0 if given file is not in a subdirectory.
 * Input: .vms/objects/de/<file> 
 * Output: 15
 * 
 * Input: foo
 * Output: 0 **/
int create_directory_path(string filepath) {
    int pos = 0;
    size_t ret = 0;
    while(ret != string::npos) {
        pos = ret;
        ret = filepath.find("/", pos+1);
        if (pos != 0) {
            mkdir(filepath.substr(0, pos+1).c_str(), 0755);
        }
    }
    return pos;
}

int get_id_from_branch(const string& branchname, string& strbuf) {
    ostringstream branch_fpath;
    branch_fpath << ".vms/branches/" << branchname;

    ifstream branch_ifs(branch_fpath.str());

    if (!branch_ifs.is_open()) {
        cerr << "Error occurred in retrieving commit pointed to by branch: unable to open file " << branch_fpath.str() << endl;
        return -1;
    }

    getline(branch_ifs, strbuf);
    branch_ifs.close();

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

    string sentinal_hash_prefix;
    string sentinal_hash_suffix;
    split_prefix_suffix(sentinal_hash, sentinal_hash_prefix, sentinal_hash_suffix, PREFIX_LENGTH);

    obj_path << ".vms/objects/" << sentinal_hash_prefix;
    make_dir(obj_path.str().c_str());
    obj_path << "/" << sentinal_hash_suffix;

    save<Commit>(sentinal, obj_path.str());

    return 0;
}

int vms_stage(const char* filepath) {

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
        cerr << "Error occurred: unable to open file " << filepath << " for staging" << endl;
        return -1;
    }
    Blob file_blob(ifilestream);

    string file_hash = file_blob.hash();

    // Puts filepath and hash into the index map and save the updated index
    index[filepath] = file_hash;
    save< map<string, string> >(index, ".vms/index");

    // Save blob in cache
    ostringstream obj_cache_path;
    obj_cache_path << ".vms/cache/" << file_hash;
    save<Blob>(file_blob, obj_cache_path.str());

    return 0;
}

int vms_unstage(const char* filepath) {
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // remove entry from the index map and save the updated index
    index.erase(string(filepath));
    save< map<string, string> >(index, ".vms/index");

    // Note: decide to not remove cache because unnecessary: will clear cache after commits
    
    return 0;
}

int vms_commit(const char* msg) {
    // TODO: Fix and be more rigorous with error handling and propagation
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    
    // Check if any tracked changes to commit, if not print and return, otherwise continue
    if (index.empty()) {
        cerr << "No changes staged to commit" << endl;
        return -1;
    }

    // Create new commit, update its internal map, and move files from cache to objects directory
    Commit commit(msg);
    cout << "commit before adding new elements" << endl;
    commit.print();

    set<string> uuid_set;     // Create set to track which uuids have been seen before so don't try to move twice
    pair<set<string>::iterator,bool> insert_ret;

    map<string,string>::iterator it;
    for (it=index.begin(); it!=index.end(); ++it) {
        // if staged is file mapped to DELETED_FILE string, remove it from commit tree
        if (it->second == DELETED_FILE) {
            commit.remove_from_map(it->first);
        } else { // puts entry to commit map and move files from cache to objects directory
            commit.put_to_map(it->first, it->second);
            insert_ret = uuid_set.insert(it->second);
            if (insert_ret.second) { // if insert succeed, then uuid has not been seen yet and can be moved.
                move_from_cache_to_objects(it->second);
            }
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
    
    string branch_fpath;
    if (get_branch_path(branch_fpath) != 0) {
        return -1;
    }
    
    create_and_write_file(branch_fpath.c_str(), commit_hash.c_str(), 0644);

    //Push formatted commit string to log and save
    stack<string> log;
    restore< stack<string> >(log, ".vms/log");

    log.push(commit.log_string());
    save< stack<string> >(log, ".vms/log");

    // Serialize and save your commit.
    ostringstream obj_path;

    string commit_hash_prefix;
    string commit_hash_suffix;
    split_prefix_suffix(commit_hash, commit_hash_prefix, commit_hash_suffix, PREFIX_LENGTH);

    obj_path << ".vms/objects/" << commit_hash_prefix;
    make_dir(obj_path.str().c_str());
    obj_path << "/" << commit_hash_suffix;

    save<Commit>(commit, obj_path.str());

    // Change permissions of generated file.
    int ret = chmod(obj_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Error occurred: failed to change permissions of commit file." << obj_path.str() << endl;
        return -1;
    }

    return 0;
}

int vms_log() {

    stack<string> log;
    restore< stack<string> >(log, ".vms/log");
    while (!log.empty()) {
        cout << "===\n";
        cout << log.top() << endl;
        log.pop();
    }

    return 0;
}

int vms_status() {

    string parent_hash; 

    if (get_parent_ref(parent_hash) != 0) {
        return -1;
    }

    // List what branches currently exist and mark current branch
    string current_branch;
    
    if (get_branch(current_branch) != 0) {
        return -1;
    }

    string branch_id;

    if (get_id_from_branch(current_branch, branch_id) != 0) {
        return -1;
    }


    stringstream status_stream;
    status_stream << "On branch " << current_branch.c_str() << "  [" << branch_id.substr(0,6) << "]\n";

    DIR *branch_dirptr = opendir(".vms/branches");
    struct dirent *branch_entry = readdir(branch_dirptr);

    bool other_branches = false;

    while (branch_entry != NULL) {

        if (strcmp(".", branch_entry->d_name) != 0 && strcmp("..", branch_entry->d_name) != 0) {

            if (strcmp(current_branch.c_str(), branch_entry->d_name) != 0) {
                
                if (!other_branches) {
                    other_branches = true;
                    status_stream << "\nAlternate branches:\n\n";
                }

                get_id_from_branch(branch_entry->d_name, branch_id);
                status_stream << "    " << branch_entry->d_name << "  [" << branch_id.substr(0,6) << "]\n";

            }

        }
        
        branch_entry = readdir(branch_dirptr);

    }
    if (other_branches) {
        status_stream << endl;
    }


    // List all files currently staged. (and list type of modification: modified, deleted)
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    map<string,string>::iterator it;

    bool staged_changes = false;

    for (it = index.begin(); it != index.end(); ++it) {
        // if new (not currently tracked)
        if (!is_tracked_file(it->first.c_str())) {

            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"vms unstage <file>...\" to unstage changes to file)\n";
                status_stream << "  (use \"vms commit <message>...\" to commit all staged changes)\n\n";
            }

            status_stream << "    new file:    " << it->first << "\n";

        } else if (it->second == DELETED_FILE) {    // if deleted (mapped to delete)
            
            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"vms unstage <file>...\" to unstage changes to file)\n";
                status_stream << "  (use \"vms commit <message>...\" to commit all staged changes)\n\n";
            }

            status_stream << "    deleted:     " << it->first << "\n";

        } else if (is_modified_tracked_file(it->first.c_str())) { // if modified

            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"vms unstage <file>...\" to unstage changes to file)\n";
                status_stream << "  (use \"vms commit <message>...\" to commit all staged changes)\n\n";
            }

            status_stream << "    modified:    " << it->first << "\n";

        } 

        // Note: the order of these checks is important
        
    }
    if (staged_changes) {
        status_stream << endl;
    }

    // List all files that have been staged and have been modified since staging (and the type of modification)

    // TODO: Improve performance of this loop.
    // TODO: Complicated logic to achieve desired behavior: simplify control logic if you can, to make things more clear.

    bool unstaged_changes = false;
    for (it = index.begin(); it != index.end(); ++it) {

        if (it->second != DELETED_FILE) {

            if (!is_valid_file(it->first.c_str())) {    // if previously staged file has been deleted, list it as deleted
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit:\n";
                    status_stream << "  (use \"vms stage <file>...\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    deleted:     " << it->first << "\n";
            } else if (!file_hash_equal_to_working_copy(it->first, it->second)) {    // if tracked file has been modified, list it as modified
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit:\n";
                    status_stream << "  (use \"vms stage <file>...\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            }

        } else { // File staged as deleted
            if (is_valid_file(it->first.c_str())) { // if file is no longer deleted and can be staged to be added again, should notify user.. by definition, it is already tracked, so maybe check if it has changes. If so, then list it as modified... if it doesn't... then problem because can't stage if no changes to stage. Easy way around it is to remove the check before staging, but bad for performance because need to do check in status loop. But this loop is fairly small, supposedly. Okay.
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit:\n";
                    status_stream << "  (use \"vms stage <file>...\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            }
        }
    }

    // list all tracked files that have not been staged and have been modified (and the type of modification)

    // Load parent commit
    Commit parent_commit;

    ostringstream parent_fpath;

    string parent_hash_prefix;
    string parent_hash_suffix;
    split_prefix_suffix(parent_hash, parent_hash_prefix, parent_hash_suffix, PREFIX_LENGTH);

    parent_fpath << ".vms/objects/" << parent_hash_prefix << "/" << parent_hash_suffix;

    restore<Commit>(parent_commit, parent_fpath.str());
    if (parent_commit.hash() != parent_hash) {
        std::cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << std::endl;
        return -1;
    }


    // Get copy of map of parent commit
    map<string, string> parent_map = parent_commit.get_map();

    for (it = parent_map.begin(); it != parent_map.end(); ++it) {
        if (!is_staged_file(it->first.c_str())) {
            if (!is_valid_file(it->first.c_str())) { // unstaged tracked file has been deleted from working directory
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit:\n";
                    status_stream << "  (use \"vms stage <file>...\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    deleted:     " << it->first << "\n";
            
            } else if (is_modified_tracked_file(it->first.c_str())) {
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit:\n";
                    status_stream << "  (use \"vms stage <file>...\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            
            }
        }
    }

    if (!unstaged_changes && !staged_changes) {
        status_stream << "No changes to staged or tracked files, working tree clean\n\n";
    } else if (unstaged_changes) {
        status_stream << endl;
    }

    // list all untracked files in this directory
    list<string> dirs;

    DIR *root_dirptr = opendir(".");
    struct dirent *root_entry = readdir(root_dirptr);

    bool untracked_files = false;

    while (root_entry != NULL) {
        if (is_valid_dir(root_entry->d_name) && strcmp(".", root_entry->d_name) != 0 && strcmp("..", root_entry->d_name) != 0 && strcmp(".vms", root_entry->d_name) != 0) {
            dirs.push_back(string(root_entry->d_name));
        }
        if (is_valid_file(root_entry->d_name) && !is_tracked_file(root_entry->d_name) && !is_staged_file(root_entry->d_name)) {
            if (!untracked_files) {
                untracked_files = true;
                status_stream << "Untracked files:\n";
                status_stream << "  (use \"vms stage <file>...\" to include file to be committed and tracked)\n\n";
            }
            status_stream << "    " << root_entry->d_name << "\n";
        }
        root_entry = readdir(root_dirptr);

    }

    if (untracked_files) {
        status_stream << endl;
    }


    if (!dirs.empty()) {
        status_stream << "Sub-directories:\n";
        list<string>::iterator dirs_iter;
        for (dirs_iter = dirs.begin(); dirs_iter != dirs.end(); ++dirs_iter) {
            status_stream << "    " << *dirs_iter << "/\n";
        }
        status_stream << endl;
    }
    
    cout << status_stream.rdbuf();
    return 0;
}


int vms_mkbranch(const char* branchname) {
    // Copy branch currently pointed to by HEAD as new branch
    string src_path;

    if (get_branch_path(src_path) != 0) {
        return -1;
    }

    ostringstream dst_path;
    dst_path << ".vms/branches/" << branchname;

    ifstream src(src_path);
    ofstream dst(dst_path.str());

    dst << src.rdbuf();
    
    src.close();
    dst.close();

    return 0;
}

int vms_mkbranch(const char* branchname, const char* commit_id) {
    ostringstream dst_path;
    dst_path << ".vms/branches/" << branchname;


    // get the full commit_id and write it to dst
    std::string id_prefix;
    std::string id_suffix;
    split_prefix_suffix(std::string(commit_id), id_prefix, id_suffix, PREFIX_LENGTH);
    
    ostringstream full_id;
    full_id << id_prefix;

    ostringstream commit_subdir;
    commit_subdir << ".vms/objects/" << id_prefix;

    const char* obj_subdir = commit_subdir.str().c_str();

    if (is_valid_dir(obj_subdir)) {
    
        DIR *dirptr = opendir(obj_subdir);
        struct dirent *entry = readdir(dirptr);

        while (entry != NULL) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (strncmp(id_suffix.c_str(), entry->d_name, id_suffix.length()) == 0) {

                    full_id << entry->d_name;

                }
            }
            entry = readdir(dirptr);
        }
    }
    
    create_and_write_file(dst_path.str().c_str(), full_id.str().c_str(), 0644);

    return 0;
}

int vms_rmbranch(const char* branchname) {
    ostringstream branch_path;
    branch_path << ".vms/branches/" << branchname;

    remove_file(branch_path.str().c_str());
    return 0;
}

int vms_info(const char* commit_id) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    cout << commit.log_string() << "\n";
    cout << commit.tracked_files_string() << endl;

    return 0;

}

int vms_info(const char* commit_id, const char* filename) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    // verify file is tracked in this commit and load its blob if it is.
    map<string, string>::iterator it;
    
    if (!commit.find_in_map_and_get_iter(string(filename), it)) {
        cerr << "File not found in commit " << commit_id << endl;
        return -1;
    }

    Blob file;
    restore_blob_from_full_id(it->second, file);

    cout << file.get_content() << endl;

    return 0;

}

int vms_checkout_branch(const char* branchname) {
    // Ask user to verify thay want to checkout the branch.
    string input;
    cout << "Checking out branch " << branchname << "...\n";
    cout << "Warning: checking out branch will overwrite all uncommitted changes in the current branch.\n"; 
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        return 0;
    }
    string commit_id;
    if (get_id_from_branch(branchname, commit_id) != 0) {
        return -1;
    }

    vms_checkout_files(commit_id.c_str());

    // check HEAD to point to this branch
    create_and_write_file(".vms/HEAD", branchname, 0644);

    // clear staging area
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    return 0;

}

int vms_checkout_files(const char* commit_id) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    map<string, string>::iterator m_elem;
    map<string, string> commit_map = commit.get_map();

    // Ask user to verify they want to checkout these files.
    cout << commit.log_string() << "\n";
    cout << "Checking out files\n";
    for (m_elem = commit_map.begin(); m_elem != commit_map.end(); m_elem++) {
        cout << "    " << m_elem->first << "\n";
    }
    cout << endl;

    string input;
    cout << "Warning: checking out files may overwrite uncommitted changes for these files in the working directory.\n";
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        return 0;
    }

    // User answered "y", so checkout files.
    for (m_elem = commit_map.begin(); m_elem != commit_map.end(); m_elem++) {
        
        create_directory_path(m_elem->first);

        Blob file;
        restore_blob_from_full_id(m_elem->second, file);

        ofstream ofs(m_elem->first);
        ofs << file.get_content();
        ofs.close();
    }

    return 0;

}

int vms_checkout_files(const char* commit_id, const int argc, char* const argv[]) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    // Validate all files to find ones that exist
    map<string, string>::iterator m_elem;
    list<string> found_files;
    for (int i = 4; i < argc; i++) {
        if (commit.find_in_map_and_get_iter(string(argv[i]), m_elem)) {
            found_files.push_back(m_elem->first);
        }
    }

    if (found_files.empty()) {
        cout << "No files match those provided. Exiting..." << endl;
        return -1;
    }

    // Ask user to verify that they want to checkout these files.
    list<string>::iterator l_elem;
    cout << commit.log_string() << "\n";
    cout << "Checking out files\n";
    for (l_elem = found_files.begin(); l_elem != found_files.end(); l_elem++) {
        cout << "    " << *l_elem << "\n";
    }
    cout << endl;

    string input;
    cout << "Warning: checking out files may overwrite uncommitted changes for these files in the working directory.\n";
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        return 0;
    }

    // User answered "y", so checkout files.
    for (l_elem = found_files.begin(); l_elem != found_files.end(); l_elem++) {
        
        commit.find_in_map_and_get_iter(*l_elem, m_elem);
        create_directory_path(m_elem->first);

        Blob file;
        restore_blob_from_full_id(m_elem->second, file);

        ofstream ofs(m_elem->first);
        ofs << file.get_content();
        ofs.close();
    }

    return 0;

}