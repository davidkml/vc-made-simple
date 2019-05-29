#include <iostream>
#include <sys/stat.h> // for mkdir
#include <sys/types.h> // for mkdir, creat
#include <sys/stat.h> // for creat
#include <fcntl.h> // for creat
#include <unistd.h> // for write, getcwd

#include <limits.h> // for realpath
#include <stdlib.h> // for realpath


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
        // cerr << "ERROR: Unable to create directory. " << strerror(errno) << endl;
        return -1;   
    }

    return 0;
}

int create_and_write_file(const char* filepath, const char* content, mode_t mode) {
    if (filepath == NULL) {
        cerr << "ERROR: Unable to create file. Provided name is not a valid string." << endl;
        return 1;
    }

    if (!is_valid_path(filepath)) {
        cerr << "ERROR: Unable to create file. Provided name is not a valid path within .vms directory." << endl;
        return 1;
    }

    int ofd = creat(filepath, mode);
    
    if (ofd == -1) {
        cerr << "ERROR: Unable to open/create file. " << strerror(errno) << endl;
        close(ofd);
        return -1;
    }

    if (content != NULL) {

        ssize_t nwrite = write(ofd, content, strlen(content));

        if(nwrite == -1) {
            cerr << "ERROR: Unable to write to file. " << strerror(errno) << endl;
            close(ofd);
            return -1;
        }

    }

    close(ofd);
    return 0;
}

int remove_file(const char* filepath) {
    if (filepath == NULL) {
        cerr << "ERROR: Unable to remove file. Provided name is not a valid string." << endl;
        return 1;
    }

    if (!is_valid_path(filepath)) {
        cerr << "ERROR: Unable to remove file. Provided name is not a valid path within .vms directory." << endl;
        return 1;
    }

    int ret = unlink(filepath);

    if (ret == -1) {
        cerr << "ERROR: Unable to remove file. " << strerror(errno) << endl;
        return -1;   
    }

    return 0;
}

int move_file(const char* src, const char* dst) {
    if (src == NULL) {
        cerr << "ERROR: Unable to move file. Provided source is not a valid string." << endl;
        return 1;
    }

    if (dst == NULL) {
        cerr << "ERROR: Unable to move file. Provided destination is not a valid string." << endl;
        return 1;
    }

    if (!is_valid_path(src)) {
        cerr << "ERROR: Unable to move file. Provided source is not a valid path within .vms directory." << endl;
        return 1;
    }

    if (!is_valid_path(dst)) {
        cerr << "ERROR: Unable to move file. Provided destination is not a valid path within .vms directory." << endl;
        return 1;
    }
    int ret = rename(src, dst);

    if (ret == -1) {
        cerr << "ERROR: Unable to move file. " << strerror(errno) << endl;
        return -1;   
    }

    return 0;

}

int normalize_relative_filepath(const char* filepath, char* buf) {
    char abs_filepath[PATH_MAX];
    if (realpath(filepath, abs_filepath) == NULL) {
        cerr << "ERROR: Unable to obtain absolute filepath of file " << filepath << ". " << strerror(errno) << endl;
        return -1;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL) {
        cerr << "ERROR: Unable to obtain current working directory. " << strerror(errno) << endl;
        return -1;
    }

    int cwd_len = strlen(cwd);
    
    if (strncmp(cwd, abs_filepath, cwd_len) != 0) {
        cerr << "ERROR: Given file " << filepath << " is not in current working directory or its subdirectories." << endl;
        return -1;
    }

    char * rel_filepath = abs_filepath + 1 + cwd_len;

    strcpy(buf, rel_filepath);
    
    return 0;
}
