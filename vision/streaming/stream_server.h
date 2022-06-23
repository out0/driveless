#ifndef __STREAM_SERVER_H
#define __STREAM_SERVER_H

#include <jetson-utils/videoOutput.h>
#include <string>
#include <thread>
#include "../log/logger.h"
#include "../control/process_handler.h"

using namespace std;

class StreamServer
{
private:
    videoOutput *outputStream;
    int listenPort;
    vector<videoOutput *> *clients;
    thread *main;
    Logger *logger;
    ProcHandler *procHandler;
    string serviceName;
    bool active;
    int listenerFd;

public:
    StreamServer(string serviceName, int listenPort, Logger *logger);
    ~StreamServer();
    void Start();
    void Stop();
    void Wait();
    bool IsActive();
    int GetListenerDescriptor();
    void OnNoAccept(string*clientIP);
    void OnStreaming(string *clientIP, int clientPort);
    bool CreateOutputStream(string *clientIP, int clientPort);
    void NewFrame(uchar3 *frame, int width, int height);
};

#endif