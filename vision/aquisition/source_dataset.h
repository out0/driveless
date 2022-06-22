#ifndef __SOURCE_DATASET_H
#define __SOURCE_DATASET_H

#include <string>
#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>

#include "source_camera.h"

class SourceCameraDatasetImpl : public SourceCamera
{
private:
    std::vector<std::string> *input;
    uint32_t width;
    uint32_t height;
    int pos;

public:
    SourceCameraDatasetImpl(uint32_t width, uint32_t height)
    {
        input = new std::vector<std::string>();
        this->width = width;
        this->height = height;
        this->pos = 0;
    }

    ~SourceCameraDatasetImpl()
    {
        delete input;
    }

    SourceCameraDatasetImpl *AddSource(std::string path)
    {
        input->push_back(path);
        return this;
    }

    virtual uint32_t GetWidth() override
    {
        return width;
    }

    virtual uint32_t GetHeight() override
    {
        return height;
    }

    virtual void *Capture(uint64_t timeout = UINT64_MAX) override
    {
        if (pos >= input->size())
        {
            pos = 0;
        }

        std::string filename = input->at(pos);
        std::ifstream file(filename, std::ifstream::binary);
        file.seekg(0, file.end);
        size_t length = static_cast<size_t>(file.tellg());
        file.seekg(0, file.beg);

        char *buffer = new char[length];
        file.read(buffer, length);
        file.close();

        this->pos++;
        return (void *)buffer;
    }

    virtual float *CaptureRGBA(uint64_t timeout = 100000)
    {
        return (float *)Capture(timeout);
    }

    virtual bool IsStreaming() override
    {
        return true;
    }

    virtual void Close() override
    {
    }

    void initWithOptions(videoOptions &options) override
    {
    }
};

#endif