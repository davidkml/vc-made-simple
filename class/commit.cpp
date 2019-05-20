#include "commit.hpp"
#include "../archive.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/compute/detail/sha1.hpp>

/* Helper function for getting hash of parent commit */
std::string get_parent_ref() {
    std::ifstream head_ifs(".vms/HEAD");
    if (!head_ifs.is_open()) {
        std::cerr << "Unable to open file .vms/HEAD" << std::endl;
        // TODO: add exception handling code
    }

    std::string branch_fpath;
    std::getline(head_ifs, branch_fpath);
    head_ifs.close();

    std::ifstream branch_ifs(branch_fpath);
    if (!branch_ifs.is_open()) {
        std::cerr << "Unable to open file " << branch_fpath << std::endl;
        // TODO: add exception handling code
    }

    std::string commit_hash;
    getline(branch_ifs, commit_hash);
    branch_ifs.close();

    return commit_hash;
}

Commit::Commit() {
    std::chrono::time_point<std::chrono::system_clock> sys_epoch;
    datetime = std::chrono::system_clock::to_time_t(sys_epoch);
}

Commit::Commit(const std::string& msg) {
    Commit parent_temp;
    std::string parent_hash = get_parent_ref();

    std::ostringstream parent_fpath;
    parent_fpath << ".vms/objects/" << parent_hash;

    restore<Commit>(parent_temp, parent_fpath.str());
    
    // get reference to parent's file_to_hash map
    std::map<std::string, std::string>& parent_map = parent_temp.get_map();

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
    std::cout << "obj string before hashing: " << oss.str() << std::endl;

    boost::compute::detail::sha1 hash(oss.str());

    return std::string(hash);
}

std::map<std::string, std::string>& Commit::get_map() {
    return file_to_hash;
}

void Commit::put_to_map(const std::string& key, const std::string& value) {
   file_to_hash[key] = value;
}