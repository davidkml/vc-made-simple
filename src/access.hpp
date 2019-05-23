/*
Collection of various file and directory access helper functions for vms
*/
#ifndef ACCESS_HPP
#define ACCESS_HPP

bool is_initialized();

bool is_staged_file(char* filepath);

bool is_valid_file(char* filepath);

bool is_valid_dir(char* dirpath);

#endif // ACCESS_HPP