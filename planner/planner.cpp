/**
 * Based on:
 * https://stackoverflow.com/questions/10403588/adding-opencv-processing-to-gstreamer-application
 */

// Include atomic std library
#include <atomic>

// Include gstreamer library
#include <gst/gst.h>
#include <gst/app/app.h>

// Include OpenCV library
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <stdio.h>
#include "stream_reader.h"

GstFlowReturn new_sample(GstAppSink *appsink, gpointer /*data*/)
{
    static int framecount = 0;

    // Get caps and frame
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    GstCaps *caps = gst_sample_get_caps(sample);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const int width = g_value_get_int(gst_structure_get_value(structure, "width"));
    const int height = g_value_get_int(gst_structure_get_value(structure, "height"));

    // Print dot every 30 frames
    if (!(framecount % 30))
    {
        g_print(".");
    }

    // Show caps on first frame
    if (!framecount)
    {
        g_print("caps: %s\n", gst_caps_to_string(caps));
    }
    framecount++;

    // Get frame data
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    printf("received a new frame with size %d x %d\n", width, height);

    // Convert gstreamer data to OpenCV Mat
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

gboolean my_bus_callback(GstBus *bus, GstMessage *message, gpointer data)
{
    // Debug message
    // g_print("Got %s message\n", GST_MESSAGE_TYPE_NAME(message));
    switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
    {
        GError *err;
        gchar *debug;

        gst_message_parse_error(message, &err, &debug);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        break;
    }
    case GST_MESSAGE_EOS:
        /* end-of-stream */
        break;
    default:
        /* unhandled message */
        break;
    }
    /* we want to be notified again the next time there is a message
     * on the bus, so returning TRUE (FALSE means we want to stop watching
     * for messages on the bus and our callback should not be called again)
     */
    return true;
}

int main(int argc, char *argv[])
{
    StreamReader *reader = new StreamReader();

    reader
        ->fromServer("10.0.0.144", 20002)
        ->withBufferSize(1)
        ->withBusCallback(my_bus_callback)
        ->withMessageSinkCallback(new_sample)
        ->withLocalPort(20002)
        ->loopReceive();

        return 0;
}