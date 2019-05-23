
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

std::string get_branch_path() {
    std::ifstream head_ifs(".vms/HEAD");
    if (!head_ifs.is_open()) {
        std::cerr << "Unable to open file .vms/HEAD" << std::endl;
        // TODO: add exception handling code
    }

    std::string branch_name;
    std::getline(head_ifs, branch_name);
    head_ifs.close();

    std::ostringstream branch_fpath;
    branch_fpath << ".vms/branches/" << branch_name;

    return branch_fpath.str();

}