#ifndef VMS_HPP
#define VMS_HPP

int vms_init();

int vms_stage(const char* filepath);

int vms_unstage(const char* filepath);

int vms_commit(const char* msg);

int vms_log();

int vms_status();

int vms_mkbranch(const char* branchname);

int vms_mkbranch(const char* branchname, const char* commit_id);

int vms_rmbranch(const char* branchname);

int vms_info(const char* commit_id);

int vms_info(const char* commit_id, const char* filename);

int vms_checkout_files(const char* commit_id);

int vms_checkout_files(const char* commit_id, const int argc, char* const argv[]);

#endif // VMS_HPP