#include "stream_client.h"

StreamClient::StreamClient(Logger *logger, std::string streamerIP, int streamerPort, std::string localIP, int localPort)
{
    this->logger = logger;
    this->streamerIP = streamerIP;
    this->streamerPort = streamerPort;
    this->localIP = localIP;
    this->localPort = localPort;
}

void StreamClient::openConnection()
{
    // create client skt
    connFd = socket(AF_INET, SOCK_STREAM, 0);

    if (connFd < 0)
    {
        logger->error("unable to open a TCP socket");
        return;
    }

    server = gethostbyname(streamerIP.c_str());

    if (server == NULL)
    {
        logger->error("host %s not found", streamerIP.c_str());
        return;
    }

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);

    serverAddr.sin_port = htons(streamerPort);

    if (connect(connFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        logger->error("can't connect to host %s on port %d", streamerIP.c_str(), streamerPort);
        return;
    }

    write(connFd, localIP.c_str(), localIP.size());
    std::string port = std::to_string(localPort);
    write(connFd, port.c_str(), port.size());

    close(connFd);
}

StreamClient::~StreamClient()
{
}

void StreamClient::RequestStreamAccess()
{
    openConnection();
}
