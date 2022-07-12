
// Include atomic std library
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include "stream_reader.h"

StreamReader::StreamReader(/* args */)
{
    local_port = 20000;
    bufferSize = 1;
    run = false;
}

StreamReader::~StreamReader()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);

    if (pipeline != nullptr)
        gst_object_unref(GST_OBJECT(pipeline));

    if (bus != nullptr)
        gst_object_unref(bus);
}

StreamReader *StreamReader::withLocalPort(int port)
{
    this->local_port = port;
    return this;
}
StreamReader *StreamReader::withBufferSize(int bufferSize)
{
    this->bufferSize = bufferSize;
    return this;
}
StreamReader *StreamReader::withBusCallback(GstBusFunc busCallback)
{
    this->busCallback = busCallback;
    return this;
}
StreamReader *StreamReader::withMessageSinkCallback(GstFlowReturn (*new_sample)(GstAppSink *appsink, gpointer user_data))
{
    this->new_sample = new_sample;
    return this;
}

StreamReader *StreamReader::fromServer(const char *ip, int port) {
    this->server_ip = ip;
    this->server_port = port;
    return this;
}

bool StreamReader::buildPipeline()
{
    gst_init(NULL, NULL);

    char confmsg[512];
    sprintf(confmsg, "udpsrc port=%d "
                     "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                     "! rtph264depay ! decodebin ! videoconvert "
                     "! appsink name=sink emit-signals=true sync=false max-buffers=%d drop=true",
            local_port, bufferSize);

    gchar *descr = g_strdup(confmsg);
    GError *error = nullptr;
    pipeline = gst_parse_launch(descr, &error);

    if (error)
    {
        g_print("could not construct pipeline: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

    if (busCallback != nullptr)
        gst_bus_add_watch(bus, busCallback, nullptr);

    return true;
}

void StreamReader::requestDataFromServer() {
    struct sockaddr_in serverAddr;

    int connFd = socket(AF_INET, SOCK_STREAM, 0);

    if (connFd < 0)
    {
        printf("unable to open a TCP socket\n");
        return;
    }

    struct hostent *server = gethostbyname(server_ip);

    if (server == NULL)
    {
        printf("host %s not found\n", server_ip);
        return;
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);

    serverAddr.sin_port = htons(server_port);

    if (connect(connFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("can't connect to host %s on port %d\n", server_ip, server_port);
        return;
    }
   
    std::string port = std::to_string(local_port);
    write(connFd, port.c_str(), port.size());
    close(connFd);
}

void StreamReader::loopReceive()
{
    run = buildPipeline();
    if (!run)
        return;

    GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    gst_app_sink_set_emit_signals((GstAppSink *)sink, true);
    gst_app_sink_set_drop((GstAppSink *)sink, true);
    gst_app_sink_set_max_buffers((GstAppSink *)sink, 1);

    GstAppSinkCallbacks callbacks = {nullptr, StreamReader::new_preroll, new_sample};
    gst_app_sink_set_callbacks(GST_APP_SINK(sink), &callbacks, nullptr, nullptr);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

    while (run)
    {
        requestDataFromServer();

        GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                     (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        busCallback(bus, msg, nullptr);
        sleep(1);
    }
}
