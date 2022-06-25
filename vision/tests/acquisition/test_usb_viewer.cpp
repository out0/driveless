#include <memory>
#include <string>
#include <iostream>

#include <stdlib.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <jetson-utils/videoSource.h>
#include <jetson-utils/videoOutput.h>
#include <jetson-utils/cudaOverlay.h>
#include <jetson-utils/cudaMappedMemory.h>
#include <jetson-inference/segNet.h>
#include <jetson-utils/glDisplay.h>
#include <source_camera_gst.h>

// v4l2-ctl --list-formats-ext -d /dev/video1

using namespace std;
using namespace cv;
using namespace chrono;

typedef uchar3 pixelType;
bool signal_received = false;

void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("received SIGINT\n");
        signal_received = true;
    }
}

int main(int argc, char **argv)
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    SourceCamera *camera = SourceCameraUSBImpl::begin()
                               ->device("/dev/video1")
                               ->withSize(640, 480)
                               ->build();

    glDisplay *display = glDisplay::Create();

    if (!display)
        printf("failed to create openGL display\n");

    while (!signal_received)
    {
        float *img = camera->CaptureRGBA();

        if (img == nullptr)
        {
            printf("failed to capture RGBA image\n");
            continue;
        }

        display->RenderOnce((float *)img, camera->GetWidth(), camera->GetHeight());

        // update status bar
        char str[256];
        sprintf(str, "Camera Viewer (%ux%u) | %.0f FPS", camera->GetWidth(), camera->GetHeight(), display->GetFPS());
        display->SetTitle(str);

        // check if the user quit
        if (display->IsClosed())
            signal_received = true;
    }

    SAFE_DELETE(camera);
    SAFE_DELETE(display);

    return 0;
}
