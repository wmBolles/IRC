#ifndef BOT
#define BOT

#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include "Server.hpp"
#include "Status.hpp"

class Bot
{
    private :
        int sock;
        int port;
        bool status;
        struct sockaddr_in serv_addr;
    public :
        Bot();
        Bot(std::string server, std::string port);
        ~Bot();
        template <typename T>
        T Convert(std::string& val)
        {
            std::stringstream tmp(val);
            T ret;
            tmp >> ret;
            return (ret);
        }
        void ParseAddress(std::string &server);
        void StartConnection();
        std::string PromtUser(std::string promt);
        void StartListen();
        void HandelBotReq();
        std::string GetDate();
        std::string GetTime();
        int         ParseData(std::string buffer);
};

#endif