#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/compute/detail/sha1.hpp>


#include "commit.hpp"
#include "archive.hpp"
#include "access.hpp"


Commit::Commit() {
    std::chrono::time_point<std::chrono::system_clock> sys_epoch;
    datetime = std::chrono::system_clock::to_time_t(sys_epoch);
}

Commit::Commit(const std::string& msg) {
    // Load parent commit
    Commit parent_commit;
    std::string parent_id;
    if (get_parent_ref(parent_id) != 0) {
        std::cerr << "Fatal error occurred in constructing new commit: unable to retrieve parent commit id. .vms directory contents likely corrupted. Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ostringstream parent_path;

    std::string parent_id_prefix;
    std::string parent_id_suffix;
    split_prefix_suffix(parent_id, parent_id_prefix, parent_id_suffix, PREFIX_LENGTH);

    parent_path << ".vms/objects/" << parent_id_prefix << "/" << parent_id_suffix;

    restore<Commit>(parent_commit, parent_path.str());
    if (parent_commit.hash() != parent_id) {
        std::cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }

    
    // copy parent's map to current object
    name_id_map = parent_commit.get_map();
    
    // set remaining fields
    datetime = time(0);
    message = msg;
    first_parent_ref = parent_id;
}

std::string Commit::hash() {
    std::ostringstream oss;
    oss << ctime(&datetime) << message << first_parent_ref << second_parent_ref;
    std::map<std::string,std::string>::iterator it;
    for (it=name_id_map.begin(); it!=name_id_map.end(); ++it) {
        oss << it->first << it->second;
    }

    boost::compute::detail::sha1 hash(oss.str());

    return std::string(hash);
}

std::string Commit::log_string() {
    std::ostringstream oss;
    oss << "commit  " << hash() << "\n";
    oss << "Date    " << ctime(&datetime);
    if (second_parent_ref != "") {
        oss << "parents " << first_parent_ref << "\n";
        oss << "        " << second_parent_ref << "\n";

    } else {
        oss << "parent  " << first_parent_ref << "\n";
    }
    oss << "\n    " << message << std::endl;

    return oss.str();
}

std::string Commit::tracked_files_string() {
    std::ostringstream oss;

    std::map<std::string,std::string>::iterator it;
    oss << "Files tracked in this commit\n\n";
    for (it=name_id_map.begin(); it!=name_id_map.end(); ++it) {
        oss << "    " << it->first << "\n";
    }

    return oss.str();
}

std::pair<std::string, std::string> Commit::parent_ids() {
    std::pair<std::string, std::string> parents;

    parents.first = first_parent_ref;
    parents.second = second_parent_ref;

    return parents;
}

std::map<std::string, std::string> Commit::get_map() {
    return name_id_map;
}

bool Commit::map_contains(const std::string& key) {
    if (name_id_map.empty()) {
        return false;
    }
    std::map<std::string, std::string>::iterator it;
    it = name_id_map.find(key);

    return it != name_id_map.end();
}

bool Commit::find_in_map_and_get_iter(const std::string& key, std::map<std::string, std::string>::iterator& it) {
    it = name_id_map.find(key);
    return it != name_id_map.end();
}

void Commit::put_to_map(const std::string& key, const std::string& value) {
    name_id_map[key] = value;
}

void Commit::remove_from_map(const std::string& key) {
    name_id_map.erase(key);
}

void Commit::set_second_parent(const std::string& commit_id) {
    second_parent_ref = commit_id;
}