#include <memory>
#include <string>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <../../acquisition/source_dataset.h>
#include "image_utils.h"

// v4l2-ctl --list-formats-ext -d /dev/video1

using namespace std;

typedef uchar3 pixelType;
bool signal_received = false;
bool test_capture();
bool test_img(SourceCameraDatasetImpl *camera, std::string path);

int main(int argc, char **argv)
{
    if (!test_capture())
        return -1;

    fprintf(stdout, "\nPASS\n");

    return 0;
}

bool test_capture()
{
    SourceCameraDatasetImpl *camera = new SourceCameraDatasetImpl(384, 216);

    camera
        ->AddSource("../../imgs/0.png")
        ->AddSource("../../imgs/1.png")
        ->AddSource("../../imgs/2.png");

    if (!test_img(camera, "../../imgs/0.png"))
        return false;
    if (!test_img(camera, "../../imgs/1.png"))
        return false;
    if (!test_img(camera, "../../imgs/2.png"))
        return false;
    if (!test_img(camera, "../../imgs/0.png"))
        return false;

    SAFE_DELETE(camera);
    return true;
}

bool test_img(SourceCameraDatasetImpl *camera, std::string path)
{
    char *file_data = (char *)ImageUtils::TestReadfile(path);
    uint32_t size = ImageUtils::TestFileSize(path);
    char *img = (char *)camera->Capture();

    for (uint32_t i = 0; i < size; i++)
    {
        if (file_data[i] != img[i])
        {
            fprintf(stderr, "mismatch at %d - file %s\n", i, path.c_str());
            return false;
        }
    }

    return true;
}