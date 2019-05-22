#ifndef VMS_HPP
#define VMS_HPP

int vms_init();

int vms_stage(char* filepath);

int vms_unstage(char* filepath);

int vms_commit(char* msg);

int vms_log();

int vms_status();

#endif // VMS_HPP