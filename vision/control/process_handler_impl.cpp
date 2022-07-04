#include "process_handler.h"
#include "../streaming/stream_server.h"

#include <jetson-utils/cudaMappedMemory.h>


class ProcHandlerImpl : public ProcHandler
{
    StreamServer *originalFrameStreamServer;
    StreamServer *segmentedFrameStreamServer;

public:
    ProcHandlerImpl(Logger *logger) {
        originalFrameStreamServer = new StreamServer("OriginalFrame", 20000, logger);
        segmentedFrameStreamServer = new StreamServer("SegmentedFrame", 20001, logger);
        originalFrameStreamServer->Start();
        segmentedFrameStreamServer->Start();
    }

    ~ProcHandlerImpl() {
        originalFrameStreamServer->Stop();
        segmentedFrameStreamServer->Stop();
        delete originalFrameStreamServer;
        delete segmentedFrameStreamServer;
    }

    void FrameSkipCaptureError() override
    {
    }
    void FrameSkipMemoryFault() override
    {
    }
    void FrameSkipNetError() override
    {
    }
    virtual void FrameSkipSegmentationOverlayError() override
    {
    }
    virtual void FrameSkipSegmentationMaskError() override
    {
    }
    virtual void FrameProcessResult(char *result_value, uint32_t width, uint32_t height) override
    {
    }
    virtual void FrameCaptured(SourceImageFormat *result_value, uint32_t width, uint32_t height) override
    {
        originalFrameStreamServer->NewFrame(result_value, width, height);
    }
    virtual void FrameSegmentationSuccess(SourceImageFormat *result_value, uint32_t width, uint32_t height) override
    {
        segmentedFrameStreamServer->NewFrame(result_value, width, height);
    }

    
};

ProcHandler *NewProcHandlerImplInstance(Logger *logger) { return new ProcHandlerImpl(logger); }