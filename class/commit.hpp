#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <string>
#include <map>
#include <boost/serialization/map.hpp>

class Commit {
    public:
        Commit();
        Commit(std::string msg);
        std::string hash();

    private:
        std::time_t datetime;
        std::string message;
        std::string first_parent_ref
        std::string second_parent_ref // for merges
        std::map<std::string, std::string>file_to_hash;

}

#endif // COMMIT_HPP