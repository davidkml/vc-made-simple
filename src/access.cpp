#include "access.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <boost/serialization/map.hpp>

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

bool is_tracked_file(char* filepath) {
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
