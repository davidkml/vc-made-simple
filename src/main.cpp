#include <iostream>

#include <sys/dir.h>

#include "access.hpp"
#include "archive.hpp"
#include "vms.hpp"
#include "utils.h"

using namespace std;

/* Helper to check if given directory path has a trailing slash */
bool has_trailing_slash(char* dirpath) {
    return (*(dirpath + strlen(dirpath) - 1) ==  '/');
}

/* Helper to remove trailing slash from a given directory path */
void remove_trailing_slash(char* dirpath) {
    *(dirpath + strlen(dirpath) - 1) = '\0';
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Usage: " <<  argv[0] << " <command> [<args>]" << endl;
        //TODO: Improve and add help text
        return -1;
    }

    if(strcmp(argv[1], "init") == 0) {
        if(is_initialized()) {
            cerr << "ERROR: Repository is already initialized" << endl;
            return -1;
        }

        return vms_init();

    } else {
        if (!is_initialized()) {
            cerr << "ERROR: Repository not initialized" << endl;
            //TODO: add help text
            return -1;
        }

        if (strcmp(argv[1], "stage") == 0 || strcmp(argv[1], "unstage") == 0) {
            if (argc < 3) {
                cerr << "ERROR: Need a file name" << endl;
                // TODO: add help text
                return -1;
            }
            
            char norm_filepath[PATH_MAX];

            if (strcmp(argv[1], "stage") == 0) {
                for (int i = 2; i < argc; i++) {

                    if (is_valid_file(argv[i])) {
                        // if argv[i] is a valid file, then normalize it
                        if (normalize_relative_filepath(argv[i], norm_filepath) != 0) { // Error, most likely given file not in cwd or its subdirectories 
                            continue;
                        }
                    } else {
                        // if argv[i] is an invalid file or it is a directory, then copy it into norm_filepath and do nothing
                        strcpy(norm_filepath, argv[i]);
                    }

                    if (is_valid_dir(norm_filepath)) {
                        if (has_trailing_slash(norm_filepath)) {
                            remove_trailing_slash(norm_filepath);
                        }

                        DIR *dirptr = opendir(norm_filepath);
                        struct dirent *entry = readdir(dirptr);

                        while (entry != NULL) {

                            char dir_filename[PATH_MAX];
                            char norm_dir_filename[PATH_MAX];
                            sprintf(dir_filename, "%s/%s", norm_filepath, entry->d_name);

                            // Only apply normalization to files that actually exist in file system.
                            // This means can still stage files that don't exist (e.g. deleted files)
                            if (is_valid_file(dir_filename)) {
                                if (normalize_relative_filepath(dir_filename, norm_dir_filename) != 0) { // Error, most likely given file not in cwd or its subdirectories 
                                entry = readdir(dirptr);
                                continue;
                                }
                            } else {
                                // file does not exist, so copy dir_filename into norm_dir_filename instead
                                strcpy(norm_dir_filename, dir_filename);
                            }

                            if (is_valid_file(norm_dir_filename) || is_tracked_file(norm_dir_filename) || is_staged_file(norm_dir_filename)) {

                                vms_stage(norm_dir_filename);

                            }

                            entry = readdir(dirptr);

                        }
                        // need to normalize before check. How would you do this? The issue is deleted files.
                    }else if (is_valid_file(norm_filepath) || is_tracked_file(norm_filepath) || is_staged_file(norm_filepath)) {

                        vms_stage(norm_filepath);

                    } else {
                        cerr << norm_filepath << " is not a valid or currently tracked file or directory." << endl;
                    }
                }

                return 0;

            } else { // argv[1] == unstage 
                for (int i = 2; i < argc; i++) {

                    if (is_valid_file(argv[i])) {
                        // if argv[i] is a valid file, then normalize it
                        if (normalize_relative_filepath(argv[i], norm_filepath) != 0) { // Error, most likely given file not in cwd or its subdirectories: file will not be unstaged
                            continue;
                        }
                    } else {
                        // if argv[i] is an invalid file or it is a directory, then copy it into norm_filepath and do nothing
                        strcpy(norm_filepath, argv[i]);
                    }


                    if (is_valid_dir(norm_filepath)) {
                        if (has_trailing_slash(norm_filepath)) {
                            remove_trailing_slash(norm_filepath);
                        }

                        DIR *dirptr = opendir(norm_filepath);
                        struct dirent *entry = readdir(dirptr);

                        while (entry != NULL) {

                            char dir_filename[BUFSIZ];
                            char norm_dir_filename[PATH_MAX];
                            sprintf(dir_filename, "%s/%s", norm_filepath, entry->d_name);

                            // Normalize file if it is valid, otherwise 
                            if (is_valid_file(dir_filename)) {
                                if (normalize_relative_filepath(dir_filename, norm_dir_filename) != 0) { // Error, most likely given file not in cwd or its subdirectories: file will not be unstaged
                                entry = readdir(dirptr);
                                continue;
                                }
                            } else {
                                // file does not exist, so copy dir_filename into norm_dir_filename instead
                                strcpy(norm_dir_filename, dir_filename);
                            }

                            if (is_staged_file(norm_dir_filename)) {

                                vms_unstage(norm_dir_filename);

                            }

                            entry = readdir(dirptr);

                        }

                    } else if (is_staged_file(norm_filepath)) {

                        vms_unstage(norm_filepath);
                    } else {
                        cerr << "No file named " << norm_filepath << " currently staged for commit" << endl;
                    }
                }
                return 0;
            }

        } else if (strcmp(argv[1], "commit") == 0) {
            if (argc < 3) {
                cerr << "ERROR: Need a commit message" << endl;
                //TODO: add help text and usage
                return -1;
            }

            return vms_commit(argv[2]);
            
        } else if (strcmp(argv[1], "log") == 0) {

            return vms_log();

        } else if (strcmp(argv[1], "status") == 0) {

            return vms_status();

        } else if (strcmp(argv[1], "checkout") == 0) {
            if (argc < 3) {
                cerr << "Usage: " << endl;
                return -1;
            }

            if (strcmp(argv[2], "branch") == 0) {
                if (argc < 4) {
                    cerr << "ERROR: Must provide branch name" << endl;
                    return -1;
                }

                if (!is_valid_branch(argv[3])) {
                    cerr << "ERROR: There is no branch named " << argv[3] << endl;
                    return -1;
                }

                string current_branch;
                if (get_branch(current_branch) != 0) {
                    return -1;
                }

                if (strcmp(argv[3], current_branch.c_str()) == 0) {
                    cout << "Already on branch " << argv[3] << endl;
                    return 0;
                }
                
                return vms_checkout_branch(argv[3]);
                
            } else if (strcmp(argv[2], "files") == 0) {
                if (argc < 4) {
                    cerr << "ERROR: Must provide commit id and files. Usage:" << endl;
                    return -1;
                }

                if (!is_valid_commit_id(argv[3])) {
                    cerr << "ERROR: Ambiguous or no matching commit id provided" << endl;
                    return -1;
                }

                // Check that files have been provided

                if (argc == 4) { // Check out all files in the commit

                    return vms_checkout_files(argv[3]);

                } else { // files given

                    return vms_checkout_files(argv[3], argc, argv);

                }

                return 0;

            } else {
                cerr << "Must checkout branch or checkout files. Usage:" << endl;
                return -1;
            }
            
        } else if (strcmp(argv[1], "mkbranch") == 0) {
            if (argc < 3) {
                cerr << "ERROR: Must provide name of branch to create" << endl;
                return -1;
            }

            if (is_valid_branch(argv[2])) {
                cout << "Branch " << argv[2] << " already exists.\n";
                cout << "  (use \"vms checkout branch " << argv[2] << "\" to move to it)" << endl;
                return 0;
            }

            if (argc == 3) { // no commit_id provided, so make branch at current commit
                return vms_mkbranch(argv[2]);

            } else { // provided commitid to create branch at
                if (!is_valid_commit_id(argv[3])) {
                    cerr << "ERROR: Branch not created. Ambiguous or no matching commit id provided" << endl;
                    return -1;
                }
                return vms_mkbranch(argv[2], argv[3]);
            }

            return 0;

        } else if (strcmp(argv[1], "rmbranch") == 0) {
            if (argc < 3) {
                cerr << "ERROR: Must provide name of branch to remove" << endl;
                return -1;
            }

            if (strcmp(argv[2], "master") == 0) {
                cerr << "ERROR: Master branch cannot be removed" << endl;
                return -1;
            }

            if (!is_valid_branch(argv[2])) {
                cout << "No branch named " << argv[2] << endl;
                return 0;
            }
            
            string current_branch;
            if (get_branch(current_branch) != 0) {
                return -1;
            }

            if (strcmp(argv[2], current_branch.c_str()) == 0) {
                cout << "Currently on branch " << argv[2] << "\n";
                cout << "  (use \"vms checkout branch <branchname>...\" to move to another branch and try again)" << endl;
                return 0;
            }

            return vms_rmbranch(argv[2]);

        } else if (strcmp(argv[1], "info") == 0) {
            if (argc < 3) {
                cerr << "ERROR: Must provide name of commit to look up" << endl;
                return -1;
            }

            if (!is_valid_commit_id(argv[2])) {
                cerr << "ERROR: Ambiguous or no matching commit id provided" << endl;
                return -1;
            }
            if (argc == 3) {
                return vms_info(argv[2]);

            } else if (argc == 4) { // provided file argument as well.
                return vms_info(argv[2], argv[3]);
            } else {
                cerr << "Only one file argument allowed. Usage:" << endl;
            }
        }
        // More commands on the way
    }
}