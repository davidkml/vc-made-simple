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

bool is_valid_branch(const char* branchname);

bool is_valid_commit_id(const char* commit_id);

int get_branch(std::string& strbuf);

int get_branch_path(std::string& strbuf);

int get_parent_ref(std::string& strbuf);

int get_id_from_branch(const std::string& branchname, std::string& strbuf);

int split_prefix_suffix(const std::string& str, std::string& prefix, std::string& suffix, unsigned int n);

const unsigned int PREFIX_LENGTH = 2;

#endif // ACCESS_HPP