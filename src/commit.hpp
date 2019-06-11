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

        std::string hash() const;
        std::string log_string() const;
        std::string tracked_files_string() const;
        std::pair<std::string, std::string> parent_ids() const;
        std::map<std::string, std::string> get_map() const;
        bool map_contains(const std::string& key) const;
        
        bool find_in_map_and_get_iter(const std::string& key, std::map<std::string, std::string>::iterator& it);
        void put_to_map(const std::string& key, const std::string& value);
        void remove_from_map(const std::string& key);
        void set_second_parent(const std::string& commit_id);
        
    private:
        std::time_t datetime;
        std::string message;
        std::string first_parent_ref;
        std::string second_parent_ref; // for merges
        std::map<std::string, std::string> name_id_map;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & datetime & message & first_parent_ref & second_parent_ref & name_id_map;
        }

};

#endif // COMMIT_HPP