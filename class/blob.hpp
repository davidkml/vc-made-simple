#ifndef BLOB_HPP
#define BLOB_HPP

#include <string>
#include <fstream>

/*
Class for storing the contents of files
*/
class Blob {
    public:
        Blob();
        Blob(std::ifstream& filestream);
        std::string hash();

    private:
        std::string content;
};

#endif // BLOB_HPP