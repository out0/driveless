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
        originalFrameStreamServer->Start();
    }

    ~ProcHandlerImpl() {
        originalFrameStreamServer->Stop();
        delete originalFrameStreamServer;
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
    virtual void FrameProcessResult(void *result_value) override
    {
    }
    virtual void FrameCaptured(SourceImageFormat *result_value, uint32_t width, uint32_t height) override
    {
        originalFrameStreamServer->NewFrame(result_value, width, height);
    }
};

ProcHandler *NewProcHandlerImplInstance(Logger *logger) { return new ProcHandlerImpl(logger); }