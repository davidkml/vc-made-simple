#include <iostream>
#include <boost/compute/detail/sha1.hpp>

#include "blob.hpp"

using namespace std;

Blob::Blob() {
    content = "";
}


Blob::Blob(ifstream& filestream) {
    setContent(filestream);
}

string Blob::hash() {
    boost::compute::detail::sha1 hash(content);
    return string(hash);
}

void Blob::setContent(ifstream& filestream) {
    content.assign(std::istreambuf_iterator<char>(filestream), std::istreambuf_iterator<char>());
}