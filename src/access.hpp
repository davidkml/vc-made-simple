/*
Collection of various file and directory access helper functions for vms
*/
#ifndef ACCESS_HPP
#define ACCESS_HPP

#include <string>

bool is_initialized();

bool is_staged_file(const char* filepath);

bool is_tracked_file(const char* filepath);

bool is_modified_tracked_file(const char* filepath);

bool file_hash_equal_to_working_copy(const std::string& filename, const std::string& hash);

bool is_valid_file(const char* filepath);

bool is_valid_dir(const char* dirpath);

std::string get_branch();

std::string get_branch_path();

std::string get_parent_ref();



#endif // ACCESS_HPP