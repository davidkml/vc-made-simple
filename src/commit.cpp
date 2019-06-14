#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/compute/detail/sha1.hpp>
#include <chrono>

#include "commit.hpp"
#include "archive.hpp"
#include "access.hpp"

using namespace std;

Commit::Commit() {
    chrono::time_point<chrono::system_clock> sys_epoch;
    datetime = chrono::system_clock::to_time_t(sys_epoch);
}

Commit::Commit(const string& msg) {
    // Load parent commit
    Commit parent_commit;
    string parent_id;
    if (get_parent_ref(parent_id) != 0) {
        cerr << "Fatal error occurred in constructing new commit: unable to retrieve parent commit id. .vms directory contents likely corrupted. Exiting..." << endl;
        exit(EXIT_FAILURE);
    }

    ostringstream parent_path;

    string parent_id_prefix;
    string parent_id_suffix;
    split_prefix_suffix(parent_id, parent_id_prefix, parent_id_suffix, PREFIX_LENGTH);

    parent_path << ".vms/objects/" << parent_id_prefix << "/" << parent_id_suffix;

    restore<Commit>(parent_commit, parent_path.str());
    if (parent_commit.hash() != parent_id) {
        cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << endl;
        exit(EXIT_FAILURE);
    }

    
    // copy parent's map to current object
    name_id_map = parent_commit.get_map();
    
    // set remaining fields
    datetime = time(0);
    message = msg;
    first_parent_ref = parent_id;
}

string Commit::hash() const{
    ostringstream oss;
    oss << ctime(&datetime) << message << first_parent_ref << second_parent_ref;
    map<string,string>::const_iterator it;
    for (it=name_id_map.begin(); it!=name_id_map.end(); ++it) {
        oss << it->first << it->second;
    }

    boost::compute::detail::sha1 hash(oss.str());

    return string(hash);
}

string Commit::log_string() const {
    ostringstream oss;
    oss << "commit  " << hash() << "\n";
    oss << "Date    " << ctime(&datetime);
    if (second_parent_ref != "") {
        oss << "parents " << first_parent_ref << "\n";
        oss << "        " << second_parent_ref << "\n";

    } else {
        oss << "parent  " << first_parent_ref << "\n";
    }
    oss << "\n    " << message << endl;

    return oss.str();
}

string Commit::tracked_files_string() const {
    ostringstream oss;

    map<string,string>::const_iterator it;
    oss << "Files tracked in this commit\n\n";
    for (it=name_id_map.begin(); it!=name_id_map.end(); ++it) {
        oss << "    " << it->first << "\n";
    }

    return oss.str();
}

pair<string, string> Commit::parent_ids() const {
    pair<string, string> parents;

    parents.first = first_parent_ref;
    parents.second = second_parent_ref;

    return parents;
}

map<string, string> Commit::get_map() const {
    return name_id_map;
}

bool Commit::map_contains(const string& key) const {
    if (name_id_map.empty()) {
        return false;
    }
    map<string, string>::const_iterator it;
    it = name_id_map.find(key);

    return it != name_id_map.end();
}

bool Commit::find_in_map_and_get_iter(const string& key, map<string, string>::iterator& it) {
    it = name_id_map.find(key);
    return it != name_id_map.end();
}

void Commit::put_to_map(const string& key, const string& value) {
    name_id_map[key] = value;
}

void Commit::remove_from_map(const string& key) {
    name_id_map.erase(key);
}

void Commit::set_second_parent(const string& commit_id) {
    second_parent_ref = commit_id;
}