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
        fprintf(stderr, "usage: %s <command> [<args>]\n", argv[0]);
        //TODO: Add output of summary documentation of commands
        return -1;
    }

    if(strcmp(argv[1], "init") == 0) {
        if(is_initialized()) {
            fprintf(stderr, "Repository is already initialized\n"
                            "  (type \"%s\" in command prompt to display a summary of available commands)\n", argv[0]);
            return -1;
        }

        return vms_init();

    } else {
        if (!is_initialized()) {
            fprintf(stderr, "Repository is not initialized\n"
                            "  (use \"%s init\" to initialize repository)\n", argv[0]);
            return -1;
        }

        if (strcmp(argv[1], "stage") == 0 || strcmp(argv[1], "unstage") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide filenames or directories\n"
                                "usage: %s %s [<filenames>] [<dirnames>]\n", argv[0], argv[1]);
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
                        fprintf(stderr, "%s is not a valid or currently tracked file or directory\n", norm_filepath);
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
                        fprintf(stderr, "No file named %s is currently staged for commit\n", norm_filepath);
                    }
                }
                return 0;
            }

        } else if (strcmp(argv[1], "commit") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide a message with your commit\n"
                                "usage: %s %s <message>\n", argv[0], argv[1]);

                return -1;
            }

            return vms_commit(argv[2]);
            
        } else if (strcmp(argv[1], "log") == 0) {

            return vms_log();

        } else if (strcmp(argv[1], "status") == 0) {

            return vms_status(argv[0]);

        } else if (strcmp(argv[1], "checkout") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must specify whether checkout branch or files\n"
                                "usage: %s %s branch <branchname>\n"
                                "usage: %s %s files <commitid> [filenames]\n", argv[0], argv[1], argv[0], argv[1]);
                return -1;
            }

            if (strcmp(argv[2], "branch") == 0) {
                if (argc < 4) {
                    fprintf(stderr, "Must provide name of branch to checkout\n"
                                    "usage: %s %s %s <branchname>\n", argv[0], argv[1], argv[2]);

                    return -1;
                }

                if (!is_valid_branch(argv[3])) {
                    fprintf(stderr, "There is no branch named \"%s\"\n"
                                    "  (use \"%s status\" to see list of available branches)\n", argv[3], argv[0]);
                    return -1;
                }

                string current_branch;
                if (get_branch(current_branch) != 0) {
                    return -1;
                }

                if (strcmp(argv[3], current_branch.c_str()) == 0) {
                    fprintf(stdout, "Already on branch %s\n", argv[3]);
                    return 0;
                }
                
                return vms_checkout_branch(argv[3]);
                
            } else if (strcmp(argv[2], "files") == 0) {
                if (argc < 4) {
                    fprintf(stderr, "Must provide id of commit to checkout and optionally files\n"
                                    "usage: %s %s %s <commitid> [filenames]\n", argv[0], argv[1], argv[2]);
                    return -1;
                }

                if (!is_valid_commit_id(argv[3])) {
                    fprintf(stderr, "Provided commit id \"%s\" generated ambiguous or no matches\n"
                                    "Please provide more characters or verify the accuracy of your input\n"
                                    "  (use \"%s log\" to see log of commits)\n", argv[3], argv[0]);
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
                fprintf(stderr, "Must specify whether checkout branch or files\n"
                                "usage: %s %s branch <branchname>\n"
                                "usage: %s %s files <commitid> [filenames]\n", argv[0], argv[1], argv[0], argv[1]);
                return -1;
            }
            
        } else if (strcmp(argv[1], "mkbranch") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide name of branch to create and optionally the id of commit to create it at\n"
                                "usage: %s %s <branchname> [commitid]\n", argv[0], argv[1]);

                return -1;
            }

            if (is_valid_branch(argv[2])) {
                fprintf(stderr, "Branch \"%s\" already exists\n"
                                "  (use \"%s checkout branch %s\" to move to it)\n", argv[2], argv[0], argv[2]);

                return 0;
            }

            if (argc == 3) { // no commit_id provided, so make branch at current commit
                return vms_mkbranch(argv[2]);

            } else { // provided commitid to create branch at
                if (!is_valid_commit_id(argv[3])) {
                    fprintf(stderr, "Branch not created\n" 
                                    "Provided commit id \"%s\" generated ambiguous or no matches\n"
                                    "Please provide more characters or verify the accuracy of your input\n"
                                    "  (use \"%s log\" to see log of commits)\n", argv[3], argv[0]);

                    return -1;
                }
                return vms_mkbranch(argv[2], argv[3]);
            }

            return 0;

        } else if (strcmp(argv[1], "rmbranch") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide name of branch to remove\n"
                                "usage: %s %s <branchname>\n", argv[0], argv[1]);

                return -1;
            }

            if (strcmp(argv[2], "master") == 0) {
                fprintf(stderr, "Master branch cannot be removed\n");
                return -1;
            }

            if (!is_valid_branch(argv[2])) {
                fprintf(stderr, "No branch named \"%s\"\n"
                                "  (use \"%s status\" to see list of available branches)\n", argv[2], argv[0]);

                return 0;
            }
            
            string current_branch;
            if (get_branch(current_branch) != 0) {
                return -1;
            }

            if (strcmp(argv[2], current_branch.c_str()) == 0) {
                fprintf(stderr, "Currently on branch %s\n"
                                "  (use \"%s checkout branch <branchname>\" to move to another branch and try again)\n", argv[2], argv[0]);
                return 0;
            }

            return vms_rmbranch(argv[2]);

        } else if (strcmp(argv[1], "info") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide name of commit to look up and optionally a filename\n"
                                "usage: %s %s <commitid> [filename]\n", argv[0], argv[1]);
                return -1;
            }

            if (!is_valid_commit_id(argv[2])) {
                fprintf(stderr, "Provided commit id \"%s\" generated ambiguous or no matches\n"
                                "Please provide more characters or verify the accuracy of your input\n"
                                "  (use \"%s log\" to see log of commits)\n", argv[2], argv[0]);
                return -1;
            }
            if (argc == 3) {
                return vms_info(argv[2]);

            } else if (argc == 4) { // provided file argument as well.
                return vms_info(argv[2], argv[3]);
            } else {
                fprintf(stderr, "May only provide a single filename argument\n"
                                "usage: %s %s <commitid> [filename]\n", argv[0], argv[1]);
            }
        } else if (strcmp(argv[1], "merge") == 0) {
            if (argc < 3) {
                fprintf(stderr, "Must provide name of branch to merge into current branch\n"
                                "usage: %s %s <branchname>\n", argv[0], argv[1]);
                return -1;
            }

            string current_branch;
            if (get_branch(current_branch) != 0) {
                return -1;
            }

            if (strcmp(argv[2], current_branch.c_str()) == 0) {
                fprintf(stderr, "Currently on branch %s: cannot merge a branch with itself\n", argv[2]);
                return -1;
            }

            if (!is_valid_branch(argv[2])) {
                fprintf(stderr, "No branch named \"%s\"\n", argv[2]);
                return -1;
            }

            return vms_merge(argv[2], current_branch.c_str());

        } else {
            fprintf(stderr, "Unknown command: \'%s %s\'\n"
                            "  (type \"%s\" in command prompt to display a summary of available commands)\n", argv[0], argv[1], argv[0]);
        }
        // More commands on the way
    }
}