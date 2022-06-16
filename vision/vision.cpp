#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <iostream>

#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <jetson-utils/videoSource.h>
#include <jetson-utils/videoOutput.h>
#include <jetson-utils/cudaOverlay.h>
#include <jetson-utils/cudaMappedMemory.h>
#include <jetson-inference/segNet.h>

#include "aquisition/source_camera_usb.h"
#include "ocupancy_grid/occupancy_grid.h"
#include "control/process_handler.h"

// https://gist.github.com/jungle-cat

using namespace std;
using namespace cv;
using namespace chrono;

typedef uchar3 pixelType;

class NeuralNetVision
{
private:
    SourceCamera *input;
    segNet *net;
    OccupancyGrid *ocgrid;
    ProcHandler *procHandler;
    bool loop_run;
    pixelType *imgMask = NULL;      // color of each segmentation class
    pixelType *imgOverlay = NULL;   // input + alpha-blended mask
    pixelType *imgComposite = NULL; // overlay with mask next to it
    pixelType *imgOutput = NULL;    // reference to one of the above three
    uint32_t visualizationFlags;

    int2 maskSize;
    int2 overlaySize;
    int2 compositeSize;
    int2 outputSize;

    // https://stackoverflow.com/questions/343219/is-it-possible-to-use-signal-inside-a-c-class

    bool allocBuffers(int width, int height, uint32_t flags)
    {
        // check if the buffers were already allocated for this size
        if (imgOverlay != NULL && width == overlaySize.x && height == overlaySize.y)
            return true;

        // free previous buffers if they exit
        CUDA_FREE_HOST(imgMask);
        CUDA_FREE_HOST(imgOverlay);
        CUDA_FREE_HOST(imgComposite);

        // allocate overlay image
        overlaySize = make_int2(width, height);

        if (flags & segNet::VISUALIZE_OVERLAY)
        {
            if (!cudaAllocMapped(&imgOverlay, overlaySize))
            {
                LogError("segnet:  failed to allocate CUDA memory for overlay image (%ux%u)\n", width, height);
                return false;
            }

            imgOutput = imgOverlay;
            outputSize = overlaySize;
        }

        // allocate mask image (half the size, unless it's the only output)
        if (flags & segNet::VISUALIZE_MASK)
        {
            maskSize = (flags & segNet::VISUALIZE_OVERLAY) ? make_int2(width / 2, height / 2) : overlaySize;

            if (!cudaAllocMapped(&imgMask, maskSize))
            {
                LogError("segnet:  failed to allocate CUDA memory for mask image\n");
                return false;
            }

            imgOutput = imgMask;
            outputSize = maskSize;
        }

        // allocate composite image if both overlay and mask are used
        if ((flags & segNet::VISUALIZE_OVERLAY) && (flags & segNet::VISUALIZE_MASK))
        {
            compositeSize = make_int2(overlaySize.x + maskSize.x, overlaySize.y);

            if (!cudaAllocMapped(&imgComposite, compositeSize))
            {
                LogError("segnet:  failed to allocate CUDA memory for composite image\n");
                return false;
            }

            imgOutput = imgComposite;
            outputSize = compositeSize;
        }

        return true;
    }

public:
    NeuralNetVision(SourceCamera *input, segNet *net, OccupancyGrid *ocgrid, ProcHandler *procHandler)
    {
        this->input = input;
        this->net = net;
        this->ocgrid = ocgrid;
        this->procHandler = procHandler;
        loop_run = true;

        SetVisualizationFlags("overlay|mask");
    }

    NeuralNetVision SetVisualizationFlags(uint32_t flags)
    {
        visualizationFlags = flags;
    }
    NeuralNetVision SetVisualizationFlags(string flags)
    {
        visualizationFlags = segNet::VisualizationFlagsFromStr(flags.c_str());
    }

    void Terminate()
    {
        loop_run = false;
    }

    void LoopUntilSignaled()
    {
        const char *ignoreClass = "void";                                          // TODO
        const segNet::FilterMode filterMode = segNet::FilterModeFromStr("linear"); // TODO

        while (loop_run)
        {
            pixelType *frame = (pixelType *)input->Capture(10000);
            if (frame == NULL)
            {
                if (!input->IsStreaming())
                    loop_run = false;

                procHandler->FrameSkipCaptureError();
                continue;
            }

            if (!allocBuffers(input->GetWidth(), input->GetHeight(), visualizationFlags))
            {
                LogError("segnet:  failed to allocate buffers\n");
                procHandler->FrameSkipMemoryFault();
                continue;
            }

            if (!net->Process(frame, input->GetWidth(), input->GetHeight(), ignoreClass))
            {
                LogError("segnet:  failed to process segmentation\n");
                procHandler->FrameSkipNetError();
                continue;
            }

            // generate overlay
            if (visualizationFlags & segNet::VISUALIZE_OVERLAY)
            {
                if (!net->Overlay(imgOverlay, overlaySize.x, overlaySize.y, filterMode))
                {
                    LogError("segnet:  failed to process segmentation overlay.\n");
                    procHandler->FrameSkipSegmentationOverlayError();
                    continue;
                }
            }

            // generate mask
            if (visualizationFlags & segNet::VISUALIZE_MASK)
            {
                if (!net->Mask(imgMask, maskSize.x, maskSize.y, filterMode))
                {
                    LogError("segnet:-console:  failed to process segmentation mask.\n");
                    procHandler->FrameSkipSegmentationMaskError();
                    continue;
                }
            }

            CUDA(cudaDeviceSynchronize());

            char *occupancyGrid = ocgrid->ComputeOcuppancyGrid(imgMask, maskSize);

            procHandler->FrameProcessResult(occupancyGrid);
        }
    }
};

extern OccupancyGrid *NewOccupancyGridImplInstance();
extern ProcHandler *NewProcHandlerImplInstance();
NeuralNetVision *visionProc;

void sig_handler(int val)
{
    if (val == SIGINT)
    {
        LogVerbose("received SIGINT\n");
        visionProc->Terminate();
    }
}

int main(int argc, char **argv)
{
    SourceCamera *camera = new SourceCameraUSBImpl("/dev/video1");
    OccupancyGrid *computeOG = NewOccupancyGridImplInstance();
    ProcHandler *procHandler = NewProcHandlerImplInstance();
    segNet *net = segNet::Create();
    visionProc = new NeuralNetVision(camera, net, computeOG, procHandler);

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        LogError("can't catch SIGINT\n");

    visionProc->LoopUntilSignaled();
    return 0;
}
