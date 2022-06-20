#include "process_handler.h"

class ProcHandlerImpl : public ProcHandler
{
public:
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
    virtual void FrameCaptured(uchar3 *result_value) override
    {
    }
};

ProcHandler *NewProcHandlerImplInstance() { return new ProcHandlerImpl(); }