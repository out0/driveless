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

class NeuralNetVision
{
private:
    SourceCamera *input;
    segNet *net;
    OccupancyGrid *ocgrid;
    ProcHandler *procHandler;
    Logger *logger;
    bool loop_run;
    SourceImageFormat *imgMask = NULL;      // color of each segmentation class
    SourceImageFormat *imgOverlay = NULL;   // input + alpha-blended mask
    SourceImageFormat *imgComposite = NULL; // overlay with mask next to it
    SourceImageFormat *imgOutput = NULL;    // reference to one of the above three
    SourceImageFormat *imgOG = NULL; // overlay with mask next to it
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
    SourceImageFormat *captureNextFrame();
    bool processSegmentation(SourceImageFormat *frame);
    void loop();

public:
    NeuralNetVision(SourceCamera *input, segNet *net, OccupancyGrid *ocgrid, ProcHandler *procHandler, Logger *logger);
    NeuralNetVision* SetVisualizationFlags(uint32_t flags);
    NeuralNetVision* SetVisualizationFlags(string flags);
    void Terminate();
    void LoopUntilSignaled();
};

#endif