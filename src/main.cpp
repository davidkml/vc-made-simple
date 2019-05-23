#include <iostream>

#include <sys/dir.h>

#include "access.hpp"
#include "archive.hpp"
#include "vms.hpp"

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

            if (strcmp(argv[1], "stage") == 0) {
                for (int i = 2; i < argc; i++) {
                    //TODO: Implement logic for stage
                    //TODO: Clean up and refactor code for staging of directories
                    if (is_valid_dir(argv[i])) {
                        if (has_trailing_slash(argv[i])) {
                            remove_trailing_slash(argv[i]);
                        }

                        DIR *dirptr = opendir(argv[i]);
                        struct dirent *entry = readdir(dirptr);

                        while (entry != NULL) {

                            char dir_filename[BUFSIZ];
                            sprintf(dir_filename, "%s/%s", argv[i], entry->d_name);

                            if (is_valid_file(dir_filename) || is_tracked_file(dir_filename)) {

                                cout << "Staging file " << dir_filename << endl;
                                vms_stage(dir_filename);

                            }

                            entry = readdir(dirptr);

                        }
                    }else if (is_valid_file(argv[i]) || is_tracked_file(argv[i])) {
                        cout << "Staging file " << argv[i] << endl;
                        vms_stage(argv[i]);
                    } else {
                        cerr << argv[i] << " is not a valid or currently tracked file or directory." << endl;
                    }
                }

                return 0;

            } else { // argv[1] == unstage 
                for (int i = 2; i < argc; i++) {
                    //TODO: Implement logic for unstage
                    if (is_valid_dir(argv[i])) {
                        if (has_trailing_slash(argv[i])) {
                            remove_trailing_slash(argv[i]);
                        }

                        DIR *dirptr = opendir(argv[i]);
                        struct dirent *entry = readdir(dirptr);

                        while (entry != NULL) {

                            char dir_filename[BUFSIZ];
                            sprintf(dir_filename, "%s/%s", argv[i], entry->d_name);

                            if (is_staged_file(dir_filename)) {

                                cout << "Unstaging file " << dir_filename << endl;
                                vms_unstage(dir_filename);

                            }

                            entry = readdir(dirptr);

                        }

                    } else if (is_staged_file(argv[i])) {
                        cout << "Unstaging file " << argv[i] << endl;
                        vms_unstage(argv[i]);
                    } else {
                        cout << "ERROR No file named " << argv[i] << "currently being tracked" << endl;
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