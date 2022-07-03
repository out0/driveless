#include "neuralnet_segmentation.h"

bool NeuralNetVision::allocBuffers(int width, int height, uint32_t flags)
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

NeuralNetVision::NeuralNetVision(SourceCamera *input, segNet *net, OccupancyGrid *ocgrid, ProcHandler *procHandler, Logger *logger)
{
    this->input = input;
    this->net = net;
    this->ocgrid = ocgrid;
    this->procHandler = procHandler;
    this->logger = logger;
    loop_run = true;

    SetVisualizationFlags("overlay|mask");
    this->ignoreClass = "void";
    this->filterMode = segNet::FilterModeFromStr("linear");
}

NeuralNetVision *NeuralNetVision::SetVisualizationFlags(uint32_t flags)
{
    visualizationFlags = flags;
    logger->info("set visualization flags to value %d", flags);
    return this;
}
NeuralNetVision *NeuralNetVision::SetVisualizationFlags(string flags)
{
    visualizationFlags = segNet::VisualizationFlagsFromStr(flags.c_str());
    logger->info("set visualization flags from %s (value %d)", flags.c_str(), visualizationFlags);
    return this;
}

void NeuralNetVision::Terminate()
{
    logger->info("process termination requested");
    loop_run = false;
}

SourceImageFormat *NeuralNetVision::captureNextFrame()
{
    SourceImageFormat *frame = (SourceImageFormat *)input->Capture(10000);
    if (frame == NULL)
    {
        if (!input->IsStreaming())
            loop_run = false;

        procHandler->FrameSkipCaptureError();
        logger->error("frame skipped by capture error");
        return nullptr;
    }

    return frame;
}

bool NeuralNetVision::allocateCudaBuffers()
{
    if (!allocBuffers(input->GetWidth(), input->GetHeight(), visualizationFlags))
    {
        procHandler->FrameSkipMemoryFault();
        logger->error("frame skipped by memory fauld");
        return false;
    }

    logger->info("buffers allocated - processing for width x height: %d x %d",
                 input->GetWidth(), input->GetHeight());

    return true;
}

bool NeuralNetVision::processSegmentation(SourceImageFormat *frame)
{
    if (!net->Process(frame, input->GetWidth(), input->GetHeight(), ignoreClass.c_str()))
    {
        procHandler->FrameSkipNetError();
        logger->error("the neuralnet failed to process segmentation");
        return false;
    }

    logger->info("neuralnet: frame processed");

    // generate overlay
    if (visualizationFlags & segNet::VISUALIZE_OVERLAY)
    {
        if (!net->Overlay(imgOverlay, overlaySize.x, overlaySize.y, filterMode))
        {
            procHandler->FrameSkipSegmentationOverlayError();
            logger->error("the neuralnet failed to process segmentation overlay");
            return false;
        }
    }

    logger->info("neuralnet: overlay");

    // generate mask
    if (visualizationFlags & segNet::VISUALIZE_MASK)
    {
        if (!net->Mask(imgMask, maskSize.x, maskSize.y, filterMode))
        {
            procHandler->FrameSkipSegmentationMaskError();
            logger->error("the neuralnet failed to process segmentation mask");
            return false;
        }
    }
    logger->info("neuralnet: mask");

    CUDA(cudaDeviceSynchronize());
    return true;
}

void NeuralNetVision::loop()
{
    SourceImageFormat *frame = captureNextFrame();
    if (frame == NULL)
        return;

    logger->info("frame captured");
    procHandler->FrameCaptured(frame, input->GetWidth(), input->GetHeight());

    // if (!processSegmentation(frame))
    //     return;

    logger->info("frame processed");

    // char *occupancyGrid = ocgrid->ComputeOcuppancyGrid(imgMask, maskSize);
    // logger->info("OG computed");

    // procHandler->FrameProcessResult(occupancyGrid);
}

void NeuralNetVision::LoopUntilSignaled()
{
    if (!allocateCudaBuffers())
        return;

    while (loop_run)
    {
        loop();
    }
}
