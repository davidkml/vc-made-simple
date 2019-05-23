
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <boost/serialization/map.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include "access.hpp"
#include "archive.hpp"
#include "commit.hpp"

using namespace std;

bool is_initialized() {
    struct stat s;
    int ret = stat(".vms", &s);
    if (ret == -1 || !S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

bool is_staged_file(char* filepath) {
    if (filepath == NULL) {
        return false;
    }

    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    map<string, string>::iterator it;
    it = index.find(string(filepath));

    return it != index.end();

}

bool is_tracked_file(char* filepath) {
    if (filepath == NULL) {
        return false;
    }

    // Load parent commit. 
    string parent_hash = get_parent_ref();
    
    ostringstream parent_fpath;
    parent_fpath << ".vms/objects/" << parent_hash;


    Commit parent_commit;
    restore<Commit>(parent_commit, parent_fpath.str());

    // Check if file found in parent commit
    return parent_commit.map_contains(string(filepath));

}

bool is_valid_file(char* filepath) {
    if (filepath == NULL) {
        return false;
    }
    struct stat s;
    int ret = stat(filepath, &s);
    if (ret == -1 || S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

bool is_valid_dir(char* dirpath) {
    if (dirpath == NULL) {
        return false;
    }
    struct stat s;
    int ret = stat(dirpath, &s);
    if (ret == -1 || !S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

std::string get_branch() {
    std::ifstream head_ifs(".vms/HEAD");
    if (!head_ifs.is_open()) {
        std::cerr << "Unable to open file .vms/HEAD" << std::endl;
        // TODO: add exception handling code
    }

    std::string current_branch;
    std::getline(head_ifs, current_branch);
    head_ifs.close();

    return current_branch;
}

std::string get_branch_path() {

    std::string branch_name = get_branch();

    std::ostringstream branch_fpath;
    branch_fpath << ".vms/branches/" << branch_name;

    return branch_fpath.str();

}

std::string get_parent_ref() {

    std::string branch_fpath = get_branch_path();

    std::ifstream branch_ifs(branch_fpath);
    if (!branch_ifs.is_open()) {
        std::cerr << "Unable to open file " << branch_fpath << std::endl;
        // TODO: add exception handling code
    }

    std::string parent_commit_hash;
    getline(branch_ifs, parent_commit_hash);
    branch_ifs.close();

    return parent_commit_hash;
}