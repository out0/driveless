#ifndef __SOURCE_CAMERA_DRIVER_USB_H
#define __SOURCE_CAMERA_DRIVER_USB_H

#include <string>
#include <iostream>
#include <string.h>
#include <jetson-utils/videoSource.h>
#include <jetson-utils/gstCamera.h>

#include "source_camera.h"

class SourceCameraBuilder
{
private:
    videoOptions *options;
    SourceCamera *source;

public:
    SourceCameraBuilder(SourceCamera *source)
    {
        options = new videoOptions();
        options->ioType = videoOptions::INPUT;
        options->zeroCopy = true;
        this->source = source;
    }

    SourceCameraBuilder *device(std::string device)
    {
        options->resource = device.c_str();
        return this;
    }

    SourceCameraBuilder *withSize(int width, int height)
    {
        options->width = width;
        options->height = height;
        return this;
    }
    SourceCameraBuilder *codec(videoOptions::Codec codec)
    {
        options->codec = codec;
        return this;
    }

    SourceCameraBuilder *gpuOnly()
    {
        options->zeroCopy = false;
        return this;
    }

    SourceCamera *build()
    {
        this->source->initWithOptions(*options);
        return this->source;
    }
};

class SourceCameraUSBImpl : public SourceCamera
{
private:
    videoSource *input;
    bool available;

public:
    SourceCameraUSBImpl()
    {
        available = false;
    }

    void initWithOptions(videoOptions &options) override
    {
        input = gstCamera::Create(options);

        available = true;

        if (!input || !input->Open())
        {
            LogError("failed to open USB camera");
            available = false;
        }
    }

    ~SourceCameraUSBImpl()
    {
        Close();
        delete input;
    }

    uint32_t GetWidth() override
    {
        if (!available)
            LogError("camera not available\n");
        return input->GetWidth();
    }

    uint32_t GetHeight() override
    {
        if (!available)
            LogError("camera not available\n");
        return input->GetHeight();
    }

    void *Capture(uint64_t timeout = UINT64_MAX) override
    {
        if (!available)
            LogError("camera not available\n");

        uchar3 *imgptr = NULL;

        if (!input->Capture(&imgptr, timeout))
        {
            // check for EOS
            if (!input->IsStreaming())
            {
                LogError("not streaming\n");
                return NULL;
            }

            LogError("failed to capture video frame\n");
            return NULL;
        }

        return (void *)imgptr;
    }

    void Close() override
    {
        if (!available)
            LogError("camera not available\n");

        input->Close();
    }

    bool IsStreaming() override
    {
        if (!available)
            LogError("camera not available\n");

        return input->IsStreaming();
    }

    static SourceCameraBuilder *begin()
    {
        SourceCamera *source = new SourceCameraUSBImpl();
        return new SourceCameraBuilder(source);
    }
};

#endif