#ifndef _IMAGE_UTILS_H
#define _IMAGE_UTILS_H

#include <string>
#include <iostream>
#include <string.h>
#include <fstream>

class ImageUtils
{
public:
    static size_t TestFileSize(std::string path)
    {
        std::ifstream file(path, std::ifstream::binary);
        file.seekg(0, file.end);
        size_t length = static_cast<size_t>(file.tellg());
        file.close();
        return length;
    }

    static void * TestReadfile(std::string filename) {
        std::ifstream file(filename, std::ifstream::binary);
        file.seekg(0, file.end);
        size_t length = static_cast<size_t>(file.tellg());
        file.seekg(0, file.beg);

        char *buffer = new char[length];
        file.read(buffer, length);
        file.close();
        return (void *)buffer;
    }
};

#endif