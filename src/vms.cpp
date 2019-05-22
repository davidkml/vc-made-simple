#include <sys/types.h> 
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

using namespace std;

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

    // Create new commit and add new entries to its internal map and move files from cache to objects directory
    Commit commit(msg);
    cout << "commit before adding new elements" << endl;
    commit.print();

    map<string,string>::iterator it;
    std::cout << "Map contains:" << std::endl;
    for (it=index.begin(); it!=index.end(); ++it) {
        commit.put_to_map(it->first, it->second);
        move_from_cache_to_objects(it->second);
    }
    cout << "commit after adding new elements" << endl;
    commit.print();


    // Iterate through cache directory, clearing it
    DIR *dirptr = opendir(".vms/cache");
    struct dirent *entry = readdir(dirptr);
    
    char cache_file_path[BUFSIZ];
    
    while (entry != NULL) {

        sprintf(cache_file_path, "%s/%s", ".vms/cache", entry->d_name);
        remove_file(cache_file_path);

        entry = readdir(dirptr);

    }

    // Clear index and save it
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    // Change position of branch pointed to by HEAD
    string commit_hash = commit.hash();

    ifstream head_ifs(".vms/HEAD");
    if (!head_ifs.is_open()) {
        cerr << "Unable to open file .vms/HEAD" << endl;
        // TODO: add exception handling code
    }

    string branch_name;
    getline(head_ifs, branch_name);
    head_ifs.close();

    std::ostringstream branch_fpath;
    branch_fpath << ".vms/branches/" << branch_name;

    create_and_write_file(branch_fpath.str().c_str(), commit_hash.c_str(), 0644);

    // Serialize and save your commit.
    ostringstream obj_path;
    obj_path << ".vms/objects/" << commit_hash;
    save<Commit>(commit, obj_path.str());
    // add chmod

    return 0;
}

int vms_log() {
    return 0;
}

int vms_status() {
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    std::map<std::string,std::string>::iterator it;
    std::cout << "Map contains:" << std::endl;
    for (it=index.begin(); it!=index.end(); ++it) {
        std::cout << it->first << " => " << it->second << std::endl;
    }

    return 0;
}
