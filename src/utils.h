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
    Utility function to remove file named filepath.
    May only remove files with .vms as the prefix.
    Returns 0 on success, or non-zero integer error code on failure.

    Examples: .vms/index, .vms/branches/master
*/
int remove_file(const char* filepath);

/* 
    Utility function for moving file named src into file named dst and possibly overwriting dst.
    May only move src and dst files with .vms as prefixes.
    Returns 0 on success, or non-zero integer error code on failure.
*/
int move_file(const char* src, const char* dst);

int normalize_relative_filepath(const char* filepath, char* buf);

#endif // UTILS_H