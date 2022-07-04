
#include "stream_server.h"
#include <jetson-utils/cudaMappedMemory.h>
#include <opencv2/core/cuda/vec_traits.hpp>


static void listener(StreamServer *streamServer);

StreamServer::StreamServer(std::string serviceName, int listenPort, Logger *logger)
{
    if (listenPort <= 0)
    {
        throw invalid_argument("listenPort must be a positive value");
    }
    if (logger == nullptr)
    {
        throw invalid_argument("please provide a valid non null logger");
    }
    this->listenPort = listenPort;

    this->clients = new vector<StreamClient *>();
    this->main = nullptr;
    this->serviceName = serviceName;
    this->active = false;
    this->logger = logger;
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
void StreamServer::OnNoAccept(const char *clientIP)
{
    logger->warning("Unable to accept connection from client %s on port %d for service %s", clientIP, listenPort, serviceName.c_str());
}

void StreamServer::OnStreaming(const char *clientIP, int clientPort)
{
    logger->info("Streaming %s to %s:%d", serviceName.c_str(), clientIP, clientPort);
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
            streamServer->OnNoAccept(inet_ntoa(clientAddr.sin_addr));
        }
        else
        {
            char *port_buffer = (char *)malloc(sizeof(char) * 1024);
            bzero(port_buffer, 1024);
            recv(connFd, port_buffer, 1024, 0);
            int clientPort = atoi(port_buffer);

            char *ip = inet_ntoa(clientAddr.sin_addr);

            if (!streamServer->CheckOutputStreamExists(ip, clientPort))
            {
                streamServer->CreateOutputStream(ip, clientPort);
                streamServer->OnStreaming(ip, clientPort);
            }
            close(connFd);
        }
    }
}

bool StreamServer::CheckOutputStreamExists(const char *clientIP, int clientPort)
{
    for (StreamClient *sc : *this->clients)
    {
        if (strcmp(sc->clientIP, clientIP) == 0 && sc->clientPort == clientPort)
        {
            std::cout << "client exists: " << clientIP << ":" << clientPort << std::endl;
            return true;
        }
    }
    return false;
}

void StreamServer::CreateOutputStream(const char *clientIP, int clientPort)
{
    int len = 1024;
    char *uri = (char *)malloc(len);
    bzero(uri, 1024);
    snprintf(uri, len, "rtp://%s:%d", clientIP, clientPort);

    videoOptions options;
    options.width = 800;
    options.height = 600;
    options.zeroCopy = true;
    options.deviceType = videoOptions::DEVICE_FILE;
    options.ioType = videoOptions::OUTPUT;
    options.codec = videoOptions::CODEC_H264;
    StreamClient *sc = new StreamClient(clientIP, clientPort, videoOutput::Create(uri, options));

    this->clients->push_back(sc);
}

void StreamServer::NewFrame(SourceImageFormat *frame, uint32_t width, uint32_t height)
{

    if (!active || this->clients->size() == 0)
        return;

    for (std::vector<StreamClient *>::iterator itr = this->clients->begin(); itr != this->clients->end(); ++itr)
    {
        StreamClient *sc = *itr;
        if (sc->stream == nullptr)
        {
            std::cout << "deleting streaming\n";
            this->clients->erase(itr);
            delete sc;
            continue;
        }

        sc->stream->Render(frame, width, height);

        char str[256];
        sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", width, height, sc->stream->GetFrameRate());
        sc->stream->SetStatus(str);

        if (!sc->stream->IsStreaming())
        {
            sc->stream->Close();
            this->clients->erase(itr);
            delete sc;
        }
    }
}
void StreamServer::NewFrame(char *frame, uint32_t width, uint32_t height)
{

    if (!active || this->clients->size() == 0)
        return;

    for (std::vector<StreamClient *>::iterator itr = this->clients->begin(); itr != this->clients->end(); ++itr)
    {
        StreamClient *sc = *itr;
        if (sc->stream == nullptr)
        {
            std::cout << "deleting streaming\n";
            this->clients->erase(itr);
            delete sc;
            continue;
        }

        uint32_t n = width * height;
        uchar3 * p = (uchar3 *)malloc(sizeof(uchar3)*n);
        
        for (uint32_t i = 0; i < n; i++) {
            p[i].x = (uchar)frame[i];
            p[i].y = 0;
            p[i].z = 0;
        }

        sc->stream->Render(p, width, height);

        char str[256];
        sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", width, height, sc->stream->GetFrameRate());
        sc->stream->SetStatus(str);

        if (!sc->stream->IsStreaming())
        {
            sc->stream->Close();
            this->clients->erase(itr);
            delete sc;
        }

        delete p;
    }
}

void StreamServer::Stop()
{
    if (!active || this->clients->size() == 0)
        return;

    active = false;
    close(listenerFd);

    for (StreamClient *sc : *this->clients)
    {
        sc->stream->Close();
        delete sc;
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