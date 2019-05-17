#ifndef UTILS_H
#define UTILS_H

/* 
    Utility function to create a new directory named dirpath.
    May only create directories with .vms as the prefix.
    Returns 0 on success, or non-zero integer error code on 
    failure.

    Examples: .vms, .vms/objects, .vms/objects/o6
*/
int make_dir(const char* dirpath);


#endif // UTILS_H