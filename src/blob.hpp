#ifndef BLOB_HPP
#define BLOB_HPP

#include <string>
#include <fstream>

namespace boost {
    namespace serialization {
        class access;
    }
}

/* Class for storing the contents of files */
class Blob {
    public:
        Blob();
        Blob(std::ifstream& filestream);
        
        std::string hash() const;
        std::string get_content() const;

        void set_content(std::ifstream& filestream);

    private:
        std::string content;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & content;
        }

};

#endif // BLOB_HPP