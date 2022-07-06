#ifndef __STREAM_SERVER_H
#define __STREAM_SERVER_H

#include <jetson-utils/videoOutput.h>
#include <string>
#include <thread>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "../log/logger.h"
#include "../control/process_handler.h"
#include "../model/vision_formats.h"

using namespace std;

class ClientConnection {
public:    
    videoOutput * stream;
    const char *clientIP;
    int clientPort;

    ClientConnection(const char * clientIP, int clientPort, videoOutput * stream) {
        this->clientIP = clientIP;
        this->clientPort = clientPort;
        this->stream = stream;
    }

    ~ClientConnection() {
        delete stream;
        delete clientIP;
    }
};

class StreamServer
{
private:
    int listenPort;
    vector<ClientConnection *> *clients;
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
    void OnNoAccept(const char *clientIP);
    void OnStreaming(const char * clientIP, int clientPort);
    bool CheckOutputStreamExists(const char * clientIP,  int clientPort);
    void CreateOutputStream(const char * clientIP, int clientPort);
    void NewFrame(SourceImageFormat *frame, uint32_t width, uint32_t height);
    void NewFrame(char *frame, uint32_t width, uint32_t height);
};

#endif