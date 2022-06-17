#ifndef __PROCESS_HANDLER_H
#define __PROCESS_HANDLER_H

class ProcHandler {
public:
    virtual void FrameSkipCaptureError() = 0;
    virtual void FrameSkipMemoryFault() = 0;
    virtual void FrameSkipNetError() = 0;
    virtual void FrameSkipSegmentationOverlayError() = 0;
    virtual void FrameSkipSegmentationMaskError() = 0;
    virtual void FrameProcessResult(void *result_value) = 0;
};

#endif