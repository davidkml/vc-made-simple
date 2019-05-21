#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>


using namespace std;

bool is_initialized() {
    struct stat s;
    int ret = stat(".vms", &s);
    if (ret == -1 || !S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

bool is_valid_file(char* filepath) {
    if (filepath == NULL) {
        return false;
    }
    struct stat s;
    int ret = stat(filepath, &s);
    if (ret == -1 || S_ISDIR(s.st_mode)) {
        return false;
    }
    return true;
}

struct option init_opts[] = {
    {"help", no_argument, NULL, 'h'},
    {"force", no_argument, NULL, 'f'}
};

struct option stage_opts[] = {
    {"help", no_argument, NULL, 'h'}
};

struct option unstage_opts[] = {
    {"help", no_argument, NULL, 'h'}
};


int main(int argc, char* argv[]) {
    int ch;

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

        while (optind < argc) {
            if ((ch = getopt_long(argc, argv, "", init_opts, NULL)) != -1) {
                // For optional option argument
                // TODO: Implement logic for each option
                switch (ch) {
                    case 'h': {
                        cout << "Help text for init" << endl;
                        break;
                    }
                    case 'f': {
                        cout << "Force reinitialize repository" << endl;
                        break;
                    }
                    default:
                        break;
                }
            } else {
                // For mandatory positional argument
                // TODO: Implement logic for init
                cout << "initializing" << endl;
                optind++;  // Skip to the next argument
            }
        }
    } else {
        if (!is_initialized()) {
            cerr << "ERROR: Repository not initialized" << endl;
            //TODO: add help text
            return -1;
        }

        if (strcmp(argv[1], "stage") == 0 || strcmp(argv[1], "unstage") == 0) {
            if (argc < 3 || !is_valid_file(argv[2])) {
                cerr << "ERROR: Missing or invalid filename" << endl;
                // TODO: add help text
                return -1;
            }
            if (strcmp(argv[1], "stage") == 0) {
                for (int i = 2; i < argc; i++) {
                    //TODO: Implement logic for stage
                    cout << "Staging file " << argv[i] << endl;
                }
            } else { // argv[1] == unstage 
                for (int i = 2; i < argc; i++) {
                    //TODO: Implement logic for unstage
                    cout << "Unstaging file " << argv[i] << endl;
                }
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
        } else if (strcmp(argv[1], "log") == 0) {
            // TODO: Implement logic for log and more options and checks
            cout << "Printing log of commits" << endl;
        } else if (strcmp(argv[1], "status") == 0) {
            // TODO: Implement logic for status
            cout << "Printing repository status" << endl;
        }
    }
}