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

#endif // VMS_HPP