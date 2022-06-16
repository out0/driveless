#ifndef __SOURCE_CAMERA_DRIVER_USB_H
#define __SOURCE_CAMERA_DRIVER_USB_H

#include <string>
#include <iostream>
#include <string.h>
#include <vector>

#include "source_camera.h"

class SourceCameraDummyImpl : public SourceCamera
{
private:
    vector<string> *input;
    uint32_t width, height;
    int pos;

public:
    SourceCameraDummyImpl(uint32_t width, height;)
    {
        input = new vector<string>();
        this.width = width;
        this.height = height;
    }

    ~SourceCameraUSBImpl()
    {
        delete input;
    }

    SourceCameraDummyImpl *AddSource(string path)
    {
        input->push_back(path);
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

        string filename = input->at(pos);
        ifstream file(filename, std::ifstream::binary);
        file.seekg(0, file.end);
        size_t length = static_cast<size_t>(file.tellg());
        file.seekg(0, file.beg);

        char * buffer = new char[length];
        file.read(buffer, length);
        file.close();
        return (void *)buffer;
    }

    virtual bool IsStreaming() override
    {
        return true;
    }

    virtual void Close() override
    {
    }
};

#endif