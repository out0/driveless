
#include "stream_server.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

static void listener(StreamServer *streamServer);

StreamServer::StreamServer(string serviceName, int listenPort, Logger *logger)
{
    if (listenPort <= 0)
    {
        throw invalid_argument("listenPort must be a positive value");
    }
    if (logger == nullptr)
    {
        throw invalid_argument("please provide a valid non null logger");
    }
    this->outputStream = outputStream;
    this->listenPort = listenPort;

    this->clients = new vector<videoOutput *>();
    this->main = nullptr;
    this->serviceName = serviceName;
    this->active = false;
}
void StreamServer::Start()
{
    listenerFd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;

    if (listenerFd < 0)
    {
        logger->error("Unable to open socket for service %s", serviceName.c_str());
        return;
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(listenPort);

    if (bind(listenerFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        logger->error("Unable to bind to port %d for service %s", listenPort, serviceName.c_str());
        return;
    }

    if (listen(listenerFd, 5) < 0)
    {
        logger->error("Unable to listen to port %d for service %s", listenPort, serviceName.c_str());
        return;
    }

    this->active = true;
    main = new thread(listener, this);
}

bool StreamServer::IsActive()
{
    return this->active;
}
int StreamServer::GetListenerDescriptor()
{
    return this->listenerFd;
}
void StreamServer::OnNoAccept(string *clientIP)
{
    logger->warning("Unable to accept connection from client %s on port %d for service %s", clientIP->c_str(), listenPort, serviceName.c_str());
}

void StreamServer::OnStreaming(string *clientIP, int clientPort)
{
    logger->info("Streaming %s to %s:%d", serviceName.c_str(), clientIP->c_str(), clientPort);
}
static void listener(StreamServer *streamServer)
{
    while (streamServer->IsActive())
    {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);

        int connFd = accept(streamServer->GetListenerDescriptor(), (struct sockaddr *)&clientAddr, &len);
        if (!streamServer->IsActive())
            return;

        if (connFd < 0)
        {
            streamServer->OnNoAccept(new string(inet_ntoa(clientAddr.sin_addr)));
        }
        else
        {
            const int MAX = 256;
            char buffer[MAX];

            bzero(buffer, MAX);
            recv(connFd, buffer, MAX, 0);
            string *clientIP = new string(buffer);

            bzero(buffer, MAX);
            recv(connFd, buffer, MAX, 0);
            int clientPort = atoi(buffer);

            streamServer->CreateOutputStream(clientIP, clientPort);
            streamServer->OnStreaming(clientIP, clientPort);
        }
    }
}

bool StreamServer::CreateOutputStream(string *clientIP, int clientPort)
{
    int len = sizeof(char) * (clientIP->size()) + 20;
    char *uri = (char *)malloc(len);
    snprintf(uri, len, "rtp://%s:%d", clientIP->c_str(), clientPort);
    videoOptions options;

    videoOutput *outputStream = videoOutput::Create(uri, options);
    this->clients->push_back(outputStream);
}

void StreamServer::NewFrame(uchar3 *frame, int width, int height)
{
    if (!active || this->clients->size() == 0)
        return;
    for (videoOutput *outputStream : *this->clients)
    {
        if (outputStream != nullptr && outputStream->IsStreaming())
        {
            outputStream->Render(frame, width, height);

            char str[256];
            sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", width, height, outputStream->GetFrameRate());
            outputStream->SetStatus(str);
        }
    }
}

void StreamServer::Stop()
{
    if (!active || this->clients->size() == 0)
        return;

    active = false;
    close(listenerFd);

    for (videoOutput *outputStream : *this->clients)
    {
        outputStream->Close();
        delete outputStream;
    }

    this->clients->clear();
}

void StreamServer::Wait()
{
    if (main == nullptr)
        return;
    main->join();
}

StreamServer::~StreamServer()
{
    if (main != nullptr)
        Stop();

    delete clients;
}