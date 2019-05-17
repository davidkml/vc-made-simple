#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"

using namespace std;

bool is_valid_path(const char* path) {
    const char* root_dir = ".vms";
    if (strncmp(root_dir, path, strlen(root_dir)) != 0) {
        return false;
    }
    return true;
}

int make_dir(const char* dirpath) {
    if (dirpath == NULL) {
        cerr << "ERROR: Unable to create directory. Provided name is not a valid string." << endl;
        return 1;
    }

    if (!is_valid_path(dirpath)) {
        cerr << "ERROR: Unable to create directory. Provided name is not a valid path within .vms directory." << endl;
        return 1;
    }

    int ret = mkdir(dirpath, 0755);

    if (ret == -1) {
        cerr << "ERROR: Unable to create directory. " << strerror(errno) << endl;
        return -1;   
    }

    return 0;
}