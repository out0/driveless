#ifndef __STREAM_SERVER_H
#define __STREAM_SERVER_H

#include <string>
#include <thread>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include "../log/logger.h"


class StreamClient
{
private:
    std::string streamerIP;
    int streamerPort;
    int localPort;
    int connFd;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    Logger *logger;
    void openConnection();

public:
    StreamClient(Logger *logger, std::string streamerIP, int streamerPort,  int localPort);
    ~StreamClient();
    void RequestStreamAccess();
};

#endif