#include <iostream>

#include "blob.hpp"

using namespace std;

Blob::Blob() {
    content = "";
}

Blob::Blob(ifstream& filestream) {
    content.assign(std::istreambuf_iterator<char>(filestream), std::istreambuf_iterator<char>());
}

string Blob::hash() {
    return content;
}