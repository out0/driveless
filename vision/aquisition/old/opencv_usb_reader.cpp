#include "opencv2/opencv.hpp"
#include <iostream>
#include <chrono>
#include <ctime>

//https://gist.github.com/jungle-cat

using namespace std;
using namespace cv;
using namespace chrono;


class SourceCamera {   
public:
    virtual uint32_t GetWidth() { return -1; }
    virtual uint32_t GetHeight() { return -1; }
    virtual void* Capture(uint64_t timeout=100000) { return NULL; }
    virtual bool IsStreaming() { return false; }
    virtual void Close() {}
};

class SourceCamera_OpenCvUSBImpl : public SourceCamera {
private:
    cv::VideoCapture device;

public:

    SourceCamera_OpenCvUSBImpl() {
        device.open(1);
    }

    virtual void Init() override {
        if(!device.isOpened()) {
    		std::cout<< "Failed to open camera." << std::endl;
	    	exit (-1);
	    }
    }

    virtual uint32_t GetWidth() override {
        return device.get(cv::CAP_PROP_FRAME_WIDTH);
    }

    virtual uint32_t GetHeight() override {
        return device.get(cv::CAP_PROP_FRAME_HEIGHT);
    }

    virtual void* Capture(uint64_t timeout=100000) override {
        Mat *frame = new Mat();
        bool res = device.read(*frame); // read a new frame from video 
        if (!res) return NULL;
        return (void *)frame;
    }

    virtual bool IsStreaming() override {
        return true;
    }

    virtual void Close() override {
        device.release();
    }
};

int main(int argc, char** argv)
{
    SourceCamera *camera = new SourceCamera_OpenCvUSBImpl();
    camera->Init();

    cout << "camera width = " << camera->GetWidth() << ", height = " << camera->GetHeight() << endl;

    Mat * frame = (Mat *)camera->Capture(1000);

    if (frame == NULL) 
    {
        cout << "Video camera is disconnected" << endl;
        exit(-1);
    }

    cv::imwrite("output.jpg", *frame);
    return 0;
}


/*
int main(int argc, char** argv)
{
    // v4l2-ctl -d0 --list-formats-ext
    // const char* gst = 	"v4l2src device=/dev/video0 ! video/x-raw, width=640, height=480, format=YUY2, framerate=30/1,pixel-aspect-ratio=1/1 ! jpegdec "
	// 			"! video/x-raw(memory:NVMM), width=640, height=480, format=BGR ! appsink"
	// 			;

    cv::VideoCapture capture(0);

    if(!capture.isOpened()) {
		std::cout<< "Failed to open camera." << std::endl;
		return (-1);
	}

    double dWidth = capture.get(cv::CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    double dHeight = capture.get(cv::CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

    cout << "camera width = " << dWidth << ", height = " << dHeight << endl;

    Mat frame;
    bool bSuccess = capture.read(frame); // read a new frame from video 

    if (!bSuccess) 
    {
        cout << "Video camera is disconnected" << endl;
        exit(-1);
    }

    cv::imwrite("output.jpg", frame);
    return 0;
}

*/