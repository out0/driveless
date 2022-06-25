#ifndef _NEURALNET_SEGMENTATION_H
#define _NEURALNET_SEGMENTATION_H

#include <memory>
#include <string>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <jetson-utils/videoSource.h>
#include <jetson-utils/videoOutput.h>
#include <jetson-utils/cudaOverlay.h>
#include <jetson-utils/cudaMappedMemory.h>
#include <jetson-inference/segNet.h>

#include "../acquisition/source_camera.h"
#include "../occupancy_grid/occupancy_grid.h"
#include "../control/process_handler.h"
#include "../log/logger.h"
#include "../segmentation/neuralnet_segmentation.h"

typedef uchar3 pixelType;

class NeuralNetVision
{
private:
    SourceCamera *input;
    segNet *net;
    OccupancyGrid *ocgrid;
    ProcHandler *procHandler;
    Logger *logger;
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

    std::string ignoreClass;
    segNet::FilterMode filterMode;

protected:
    bool allocBuffers(int width, int height, uint32_t flags);
    bool allocateCudaBuffers();
    pixelType *captureNextFrame();
    bool processSegmentation(pixelType *frame);
    void loop();

public:
    NeuralNetVision(SourceCamera *input, segNet *net, OccupancyGrid *ocgrid, ProcHandler *procHandler, Logger *logger);
    NeuralNetVision* SetVisualizationFlags(uint32_t flags);
    NeuralNetVision* SetVisualizationFlags(string flags);
    void Terminate();
    void LoopUntilSignaled();
};

#endif