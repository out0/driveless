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

#include "source_camera_gst.h"

// https://gist.github.com/jungle-cat
// v4l2-ctl --list-formats-ext -d /dev/video1

using namespace std;
using namespace cv;
using namespace chrono;

typedef uchar3 pixelType;

int main(int argc, char **argv)
{
    SourceCamera *camera = SourceCameraUSBImpl::begin()
                               ->device("/dev/video1")
                               ->withSize(640, 480)
                               ->build();

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

    cv::Mat mask_image_bgr, mask_image_rgb, original_image_bgr, original_image_rgb;
    original_image_rgb = cv::Mat(camera->GetHeight(), camera->GetWidth(), CV_8UC3, imgInput);
    original_image_bgr = cv::Mat(camera->GetHeight(), camera->GetWidth(), CV_8UC3);
    cv::cvtColor(original_image_rgb, original_image_bgr, cv::COLOR_RGB2BGR);
    std::string img_name = std::string("frame.png");
    cv::imwrite(img_name, original_image_bgr);

    return 0;
}
