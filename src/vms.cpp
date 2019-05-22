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

    // Puts filepath and hash into the index map
    index[filepath] = file_hash;
    save< map<string, string> >(index, ".vms/index");

    // Save it in a temporary folder. This serves as cache.
    ostringstream obj_cache_path;
    obj_cache_path << ".vms/cache/" << file_hash;
    save<Blob>(file_blob, obj_cache_path.str());

    return 0;
}

int vms_unstage(char* filepath) {
    return 0;
}

int vms_commit(char* msg) {
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
