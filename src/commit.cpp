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
    std::string parent_hash = get_parent_ref();
    std::ostringstream parent_fpath;

    std::string parent_hash_prefix;
    std::string parent_hash_suffix;
    split_prefix_suffix(parent_hash, parent_hash_prefix, parent_hash_suffix, PREFIX_LENGTH);

    parent_fpath << ".vms/objects/" << parent_hash_prefix << "/" << parent_hash_suffix;

    restore<Commit>(parent_commit, parent_fpath.str());
    
    // get reference to parent's file_to_hash map
    std::map<std::string, std::string>& parent_map = parent_commit.get_map();

    // invoke copy constructor on parent's map to current obj 
    file_to_hash = parent_map; 
    
    // set remaining fields
    datetime = time(0);
    message = msg;
    first_parent_ref = parent_hash;
}

std::string Commit::hash() {
    std::ostringstream oss;
    oss << ctime(&datetime) << message << first_parent_ref << second_parent_ref;
    std::map<std::string,std::string>::iterator it;
    for (it=file_to_hash.begin(); it!=file_to_hash.end(); ++it) {
        oss << it->first << it->second;
    }

    boost::compute::detail::sha1 hash(oss.str());

    return std::string(hash);
}

std::string Commit::log_string() {
    std::ostringstream oss;
    oss << "commit  " << hash() << std::endl;
    oss << "Date    " << ctime(&datetime);
    if (second_parent_ref != "") {
        oss << "parents " << first_parent_ref << std::endl;
        oss << "        " << second_parent_ref << std::endl;

    } else {
        oss << "parent  " << first_parent_ref << std::endl;
    }
    oss << "\n    " << message << std::endl;

    return oss.str();
}

std::map<std::string, std::string>& Commit::get_map() {
    return file_to_hash;
}

void Commit::put_to_map(const std::string& key, const std::string& value) {
    file_to_hash[key] = value;
}

std::map<std::string, std::string>::iterator Commit::find_in_map(const std::string& key) {
    return file_to_hash.find(key);
}

void Commit::remove_from_map(const std::string& key) {
    file_to_hash.erase(key);
}

bool Commit::map_contains(const std::string& key) {
    if (file_to_hash.empty()) {
        return false;
    }
    std::map<std::string, std::string>::iterator it;
    it = file_to_hash.find(key);

    return it != file_to_hash.end();
}


void Commit::print() {
    char* dt = ctime(&datetime);
    std::cout << "Time: " << dt;
    std::cout << "Message: " << message << std:: endl;
    std::cout << "First Parent: " << first_parent_ref << std:: endl;
    std::cout << "Second Parent: " << second_parent_ref << std::endl; 
    
    std::map<std::string,std::string>::iterator it;
    std::cout << "Map contains:" << std::endl;
    for (it=file_to_hash.begin(); it!=file_to_hash.end(); ++it) {
        std::cout << it->first << " => " << it->second << std::endl;
    }
}
