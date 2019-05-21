#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <string>
#include <ctime>
#include <map>
#include <boost/serialization/map.hpp>

namespace boost {
    namespace serialization {
        class access;
    }
}

class Commit {
    public:
        Commit();
        Commit(const std::string& msg);
        std::string hash();
        void print(); // delete after testing
        std::map<std::string, std::string>& get_map();
        void put_to_map(const std::string& key, const std::string& value);

    private:
        std::time_t datetime;
        std::string message;
        std::string first_parent_ref;
        std::string second_parent_ref; // for merges
        std::map<std::string, std::string> file_to_hash;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & datetime & message & first_parent_ref & second_parent_ref & file_to_hash;
        }

};

#endif // COMMIT_HPP