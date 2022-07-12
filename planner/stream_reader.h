// #ifdef _STREAM_HEADER_H
// #define _STREAM_HEADER_H

// Include gstreamer library
#include <gst/gst.h>
#include <gst/app/app.h>
#include <stdio.h>

class StreamReader
{
private:
    const char * server_ip;
    int server_port;
    int local_port;
    int bufferSize;
    GstElement *pipeline;
    GstBus *bus;
    GstBusFunc busCallback;
    GstFlowReturn (*new_sample)(GstAppSink *appsink, gpointer user_data);

    bool buildPipeline();
    gboolean bus_callback_handler(GstBus *bus, GstMessage *message, gpointer data);
    bool run;

    void requestDataFromServer();

    static GstFlowReturn new_preroll(GstAppSink * /*appsink*/, gpointer /*data*/)
    {
        return GST_FLOW_OK;
    }

public:
    StreamReader *fromServer(const char *ip, int port);
    StreamReader *withLocalPort(int port);
    StreamReader *withBufferSize(int bufferSize);
    StreamReader *withBusCallback(GstBusFunc busCallback);
    StreamReader *withMessageSinkCallback(GstFlowReturn (*new_sample)(GstAppSink *appsink, gpointer user_data));
    void loopReceive();
    ;

    StreamReader(/* args */);
    ~StreamReader();
};

// #endif