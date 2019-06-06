#ifndef VMS_HPP
#define VMS_HPP

int vms_init();

int vms_stage(const char* filepath);

int vms_unstage(const char* filepath);

int vms_commit(const char* msg);

int vms_log();

int vms_status(const char* arg0);

int vms_mkbranch(const char* branchname);

int vms_mkbranch(const char* branchname, const char* commit_id);

int vms_rmbranch(const char* branchname);

int vms_info(const char* commit_id);

int vms_info(const char* commit_id, const char* filename);

int vms_checkout_branch(const char* branchname);

int vms_checkout_files(const char* commit_id);

int vms_checkout_files(const char* commit_id, const int argc, char* const argv[]);

int vms_merge(const char* given_branch, const char* current_branch);

#endif // VMS_HPP