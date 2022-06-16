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

#include "camera_drivers/source_camera_usb.h"

// https://gist.github.com/jungle-cat

using namespace std;
using namespace cv;
using namespace chrono;

typedef uchar3 pixelType;

int main(int argc, char **argv)
{

    SourceCamera *camera = new SourceCameraDummyImpl("/dev/video1");

    pixelType *imgInput = (pixelType *)camera->Capture(100000);

    if (imgInput == nullptr)
    {
        // check for EOS
        if (!camera->IsStreaming())
        {
            LogError("not streaming\n");
            exit(-1);
        }

        LogError("segnet:  failed to capture video frame\n");
        exit(-1);
    }

    cout << "camera width = " << camera->GetWidth() << ", height = " << camera->GetHeight() << endl;

    // cv::imwrite("output.jpg", imgInput);
    return 0;
}

