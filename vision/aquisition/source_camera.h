#ifndef __SOURCE_CAMERA_H
#define __SOURCE_CAMERA_H

#include <iostream>
#include <string>
#include <string.h>
#include <jetson-utils/videoSource.h>

class SourceCamera {   
public:
    virtual uint32_t GetWidth() = 0;
    virtual uint32_t GetHeight() = 0;
    virtual void* Capture(uint64_t timeout=100000) = 0;
    virtual bool IsStreaming() = 0;
    virtual void Close() = 0;
};

#endif