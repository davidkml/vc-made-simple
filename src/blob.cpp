#include <iostream>
#include <boost/compute/detail/sha1.hpp>

#include "blob.hpp"

using namespace std;

Blob::Blob() {
    content = "";
}


Blob::Blob(ifstream& filestream) {
    set_content(filestream);
}

string Blob::hash() const {
    boost::compute::detail::sha1 hash(content);
    return string(hash);
}

string Blob::get_content() const {
    return content;
}

void Blob::set_content(ifstream& filestream) {
    content.assign(istreambuf_iterator<char>(filestream), istreambuf_iterator<char>());
}