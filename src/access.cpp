
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>


#include <map>
#include <boost/serialization/map.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include "access.hpp"
#include "archive.hpp"
#include "blob.hpp"
#include "commit.hpp"

using namespace std;

int restore_parent_commit(Commit& commit) {
    string parent_hash;

    if (get_parent_ref(parent_hash) != 0) {
        return -1;
    }

    ostringstream parent_fpath;

    string parent_hash_prefix;
    string parent_hash_suffix;
    split_prefix_suffix(parent_hash, parent_hash_prefix, parent_hash_suffix, PREFIX_LENGTH);

    parent_fpath << ".vms/objects/" << parent_hash_prefix << "/" << parent_hash_suffix;

    restore<Commit>(commit, parent_fpath.str());

    if (commit.hash() != parent_hash) {
        std::cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << std::endl;
        return -1;
    }

    return 0;
}

bool is_initialized() {
    struct stat s;
    int ret = stat(".vms", &s);
    if (ret == -1 || !S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

bool is_staged_file(const char* filepath) {
    if (filepath == NULL) {
        return false;
    }

    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    map<string, string>::iterator it;
    it = index.find(string(filepath));

    return it != index.end();

}

bool is_tracked_file(const char* filepath) {
    if (filepath == NULL) {
        return false;
    }

    Commit parent_commit;

    restore_parent_commit(parent_commit);

    return parent_commit.map_contains(string(filepath));

}

bool is_modified_tracked_file(const char* filepath) {

    if (filepath == NULL) {
        return false;
    }

    Commit parent_commit;

    restore_parent_commit(parent_commit);

    map<string, string> parent_map = parent_commit.get_map();

    map<string, string>::iterator it;
    it = parent_map.find(string(filepath));

    if (it == parent_map.end()) {
        return true;    // Not being tracked, so by definition is modified relative to "tracked version"
    } 

    // get its hash and compare it with hash of file of same name in working directory. If it is not equal, then is modified
    return !file_hash_equal_to_working_copy(string(filepath), it->second);
}

bool file_hash_equal_to_working_copy(const std::string& filename, const std::string& hash) {
    ifstream file_ifs(filename);
    Blob file_contents(file_ifs);

    return file_contents.hash() == hash;
}

bool is_valid_file(const char* filepath) {
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

bool is_valid_dir(const char* dirpath) {
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

bool is_valid_branch(const char* branchname) {
    DIR *dirptr = opendir(".vms/branches");
    struct dirent *entry = readdir(dirptr);

    while (entry != NULL) {
        if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
            if (strcmp(branchname, entry->d_name) == 0) {

                return true;

            }
        }
        entry = readdir(dirptr);

    }

    return false;

}

bool is_valid_commit_id(const char* commit_id) {

    if (strlen(commit_id) <= PREFIX_LENGTH) {
        return false;
    }

    std::string id_prefix;
    std::string id_suffix;
    split_prefix_suffix(std::string(commit_id), id_prefix, id_suffix, PREFIX_LENGTH);

    ostringstream oss;
    oss << ".vms/objects/" << id_prefix;

    const char* obj_subdir = oss.str().c_str();
    
    
    if (is_valid_dir(obj_subdir)) {
    
        DIR *dirptr = opendir(obj_subdir);
        struct dirent *entry = readdir(dirptr);

        int n_matches = 0;

        while (entry != NULL) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (strncmp(id_suffix.c_str(), entry->d_name, id_suffix.length()) == 0) {

                    n_matches++;

                }
            }
            entry = readdir(dirptr);
        }

        // must be exactly one match; else false
        return n_matches == 1;

    } // if not a valid dir, then cannot possibly exist, so return 0

    return false;
}

int get_branch(string& strbuf) {
    ifstream head_ifs(".vms/HEAD");
    if (!head_ifs.is_open()) {
        cerr << "Error occurred in retrieving branch: unable to open file .vms/HEAD" << endl;
        return -1;
    }

    getline(head_ifs, strbuf);
    head_ifs.close();

    return 0;
}

int get_branch_path(string& strbuf) {

    string branch_name;
    if (get_branch(branch_name) != 0) {
        return -1;
    }

    ostringstream branch_fpath;
    branch_fpath << ".vms/branches/" << branch_name;
    strbuf = branch_fpath.str();

    return 0;

}

int get_parent_ref(string& strbuf) {

    string branch_fpath;
    
    if (get_branch_path(branch_fpath) != 0) {
        return -1;
    }

    std::ifstream branch_ifs(branch_fpath);
    if (!branch_ifs.is_open()) {
        std::cerr << "Error occurred in retrieving parent commit id: unable to open file " << branch_fpath << std::endl;
        return -1;
    }

    getline(branch_ifs, strbuf);
    branch_ifs.close();

    return 0;
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


int split_prefix_suffix(const std::string& str, std::string& prefix, std::string& suffix, int n) {
    if (n > str.length() - 1) {
        return -1;
    }
    prefix = str.substr(0, n);
    suffix = str.substr(n);
    return 0;
}