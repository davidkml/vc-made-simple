#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>

/* 
    Utility function to create a new directory named dirpath.
    May only create directories with .vms as the prefix.
    Returns 0 on success, or non-zero integer error code on 
    failure.

    Examples: .vms, .vms/objects, .vms/objects/o6
*/
int make_dir(const char* dirpath);

/* 
    Utility function to create (or overwrite if already exists) a new file named filepath with provided contents and permissions.
    May create (or overwrite) file with no contents by passing NULL to "content" function parameter
    May only create files with .vms as the prefix.
    Returns 0 on success, or non-zero integer error code on failure.

    Examples: .vms/index, .vms/branches/master
*/
int create_and_write_file(const char* filepath, const char* content, mode_t mode);

/* 
    Utility function to create (or overwrite if already exists) an empty file named filepath with provided permissions.
    May only create files with .vms as the prefix.
    Returns 0 on success, or non-zero integer error code on failure.

    Examples: .vms/index, .vms/branches/master
*/
int create_empty_file(const char* filepath, mode_t mode);
#endif // UTILS_H