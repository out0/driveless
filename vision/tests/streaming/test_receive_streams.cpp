#include "../../log/logger.h"
#include "../../streaming/stream_client.h"
#include <iostream>
#include <string>

extern Logger *NewDebugLoggerInstance();

int main (int argc, char **argv) {
    if(argc < 5)
    {
        cerr<<"Syntax : ./client <host> <port> <local_ip> <local_port>"<<endl;
        return 0;
    }
    
    int serverPort = atoi(argv[2]);
    
    if((serverPort > 65535) || (serverPort < 2000))
    {
        cerr<<"Please enter port number between 2000 - 65535"<<endl;
        return 0;
    }       

    int localPort = atoi(argv[4]);
    
    if((localPort > 65535) || (localPort < 2000))
    {
        cerr<<"Please enter local_port number between 2000 - 65535"<<endl;
        return 0;
    }       

    Logger *logger = NewDebugLoggerInstance();
    StreamClient *client = new StreamClient(logger, std::string(argv[1]), serverPort, std::string(argv[3]), localPort);
    client->RequestStreamAccess();
}