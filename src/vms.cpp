#include <map>
#include <boost/serialization/map.hpp>

#include <stack>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/stack.hpp>

#include <sstream>

#include "vms.hpp"
#include "utils.h"
#include "archive.hpp"
#include "commit.hpp"

using namespace std;
int vms_init() {

    // Initialize directories
    int ret = make_dir(".vms");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/objects");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/branches");
    if (ret != 0) {
        return -1;
    }

    // Initialize files
    map<string, string> index;
    save< map<string, string> >(index, ".vms/index");

    stack<string> log;
    save< stack<string> >(log, ".vms/log");

    // Initialize initial commit
    Commit sentinal;
    string sentinal_hash = sentinal.hash();
    cout << sentinal_hash << endl;

    create_and_write_file(".vms/HEAD", "master", 0644);
    create_and_write_file(".vms/branches/master", sentinal_hash.c_str(), 0644);

    ostringstream obj_path;
    obj_path << ".vms/objects/" << sentinal_hash;
    save<Commit>(sentinal, obj_path.str());

    return 0;
}

int vms_stage(char* filepath) {
    return 0;
}

int vms_unstage(char* filepath) {
    return 0;
}

int vms_commit(char* msg) {
    return 0;
}

int vms_log() {
    return 0;
}

int vms_status() {
    return 0;
}
