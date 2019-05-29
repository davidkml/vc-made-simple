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
        cout << "initializing" << endl;
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
                    //TODO: Implement logic for stage
                    //TODO: Clean up and refactor code for staging of directories

                    if (is_valid_file(argv[i])) {
                        // if argv[i] is a valid file, then normalize it
                        if (normalize_relative_filepath(argv[i], norm_filepath) != 0) { // Error, most likely given file not in cwd or its subdirectories 
                            cout << argv[i] << " not staged" << endl;
                            continue;
                        }
                    } else {
                        // if argv[i] is an invalid file or it is a directory, then copy it into normalized and do nothing
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
                                cout << dir_filename << " not staged" << endl;
                                entry = readdir(dirptr);
                                continue;
                                }
                            } else {
                                // file does not exist, so copy dir_filename into norm_dir_filename instead
                                strcpy(norm_dir_filename, dir_filename);
                            }

                            if (is_valid_file(norm_dir_filename) || is_tracked_file(norm_dir_filename) || is_staged_file(norm_dir_filename)) {

                                cout << "Staging file " << norm_dir_filename << endl;
                                vms_stage(norm_dir_filename);

                            }

                            entry = readdir(dirptr);

                        }
                        // need to normalize before check. How would you do this? The issue is deleted files.
                    }else if (is_valid_file(norm_filepath) || is_tracked_file(norm_filepath) || is_staged_file(norm_filepath)) {

                        cout << "Staging file " << norm_filepath << endl;
                        vms_stage(norm_filepath);

                    } else {
                        cerr << norm_filepath << " is not a valid or currently tracked file or directory." << endl;
                    }
                }

                return 0;

            } else { // argv[1] == unstage 
                for (int i = 2; i < argc; i++) {
                    //TODO: Implement logic for unstage

                    if (is_valid_file(argv[i])) {
                        // if argv[i] is a valid file, then normalize it
                        if (normalize_relative_filepath(argv[i], norm_filepath) != 0) { // Error, most likely given file not in cwd or its subdirectories 
                            cout << argv[i] << " not staged" << endl;
                            continue;
                        }
                    } else {
                        // if argv[i] is an invalid file or it is a directory, then copy it into normalized and do nothing
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
                                if (normalize_relative_filepath(dir_filename, norm_dir_filename) != 0) { // Error, most likely given file not in cwd or its subdirectories 
                                cout << dir_filename << " not staged" << endl;
                                entry = readdir(dirptr);
                                continue;
                                }
                            } else {
                                // file does not exist, so copy dir_filename into norm_dir_filename instead
                                strcpy(norm_dir_filename, dir_filename);
                            }

                            if (is_staged_file(norm_dir_filename)) {

                                cout << "Unstaging file " << norm_dir_filename << endl;
                                vms_unstage(norm_dir_filename);

                            }

                            entry = readdir(dirptr);

                        }

                    } else if (is_staged_file(norm_filepath)) {

                        cout << "Unstaging file " << norm_filepath << endl;
                        vms_unstage(norm_filepath);
                    } else {
                        cout << "ERROR No file named " << norm_filepath << "currently being tracked" << endl;
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
            //TODO: Implement logic for commit
            //TODO: Add more options and checks
            cout << "Committing with message: " << argv[2] << endl;
            return vms_commit(argv[2]);
            
        } else if (strcmp(argv[1], "log") == 0) {
            // TODO: Implement logic for log and more options and checks
            cout << "Printing log of commits" << endl;
            return vms_log();

        } else if (strcmp(argv[1], "status") == 0) {
            // TODO: Implement logic for status
            cout << "Printing repository status" << endl;
            return vms_status();

        }
        // More commands on the way
    }
}