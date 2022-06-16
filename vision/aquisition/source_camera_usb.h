#ifndef __SOURCE_CAMERA_DRIVER_USB_H
#define __SOURCE_CAMERA_DRIVER_USB_H

#include <string>
#include <iostream>
#include <string.h>
#include <jetson-utils/videoSource.h>

#include "source_camera.h"

class SourceCameraUSBImpl : public SourceCamera
{
private:
    videoSource *input;
    bool available;

public:
    SourceCameraUSBImpl() {
        available = false;
    }

    SourceCameraUSBImpl(std::string device)
    {
        const videoOptions options;
        input = videoSource::Create(("v4l2://" + device).c_str(), options);
        available = true;

        if (!input)
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

    virtual uint32_t GetWidth() override
    {
        if (!available)
            LogError("camera not available\n");
        return input->GetWidth();
    }

    virtual uint32_t GetHeight() override
    {
        if (!available)
            LogError("camera not available\n");
        return input->GetHeight();
    }

    virtual void *Capture(uint64_t timeout = UINT64_MAX) override
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

    virtual bool IsStreaming() override
    {
        if (!available)
            LogError("camera not available\n");

        return input->IsStreaming();
    }

    virtual void Close() override
    {
        if (!available)
            LogError("camera not available\n");

        input->Close();
    }
};


#endif