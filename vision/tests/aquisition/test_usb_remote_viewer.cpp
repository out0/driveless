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

    videoOptions options;
    videoOutput *outputStream = videoOutput::Create("rtp://10.0.0.161:20000", options);
    videoOutput *outputStream2 = videoOutput::Create("rtp://10.0.0.161:20001", options);

    while (!signal_received)
    {
        uchar3 *frame = (uchar3 *)camera->Capture();

        if (frame == nullptr)
        {
            printf("failed to capture RGBA image\n");
            continue;
        }

        if (outputStream != NULL)
        {
            outputStream->Render(frame, camera->GetWidth(), camera->GetHeight());

            char str[256];
            sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", camera->GetWidth(), camera->GetHeight(), outputStream->GetFrameRate());
            outputStream->SetStatus(str);

            // check if the user quit
            if (!outputStream->IsStreaming())
                break;
        }

        if (outputStream2 != NULL)
        {
            outputStream2->Render(frame, camera->GetWidth(), camera->GetHeight());

            char str[256];
            sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", camera->GetWidth(), camera->GetHeight(), outputStream2->GetFrameRate());
            outputStream2->SetStatus(str);
        }
    }

    SAFE_DELETE(camera);
    SAFE_DELETE(outputStream);
    SAFE_DELETE(outputStream2);

    return 0;
}
