#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <iostream>
#include <string>
#include <fstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>


template <class T>
void save(const T& obj, const std::string& filepath) {
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) {
        std::cerr << "ERROR: File could not be opened." << std::endl;
        return;
    }

    {
        boost::iostreams::filtering_ostreambuf fos_buf;

        fos_buf.push(boost::iostreams::zlib_compressor(boost::iostreams::zlib::best_compression));
        fos_buf.push(ofs);

        boost::archive::binary_oarchive boa(fos_buf);
        boa << obj;
    }
}

template <class T>
void restore(T& obj, const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "ERROR: File could not be opened." << std::endl;
        return;
    }

    {
        boost::iostreams::filtering_istreambuf fis_buf;

        fis_buf.push(boost::iostreams::zlib_decompressor());
        fis_buf.push(ifs);

        boost::archive::binary_iarchive bia(fis_buf);
        bia >> obj;
    }
}

#endif // ARCHIVE_HPP