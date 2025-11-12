#include "Bot.hpp"

void Bot::StartConnection()
{
    std::string pass = this->PromtUser("Password : ");
    std::string nickname = this->PromtUser("Nickname : ");

    Status("Sending Passowrd ...", 0);
    std::string buffer = "PASS " + pass + "\r\n";
    if (send(this->sock, buffer.c_str(), buffer.size(), 0) == -1)
        throw std::runtime_error("Failed to send PASS");
    sleep(1);
    buffer = "NICK " + nickname + "\r\n";
    Status("Sending Nickname ...", 0);
    if (send(this->sock, buffer.c_str(), buffer.size(), 0) == -1)
        throw std::runtime_error("Failed to send NICK");
    sleep(1);
    buffer = "USER " + nickname + " 0 * BOT" + "\r\n";
    Status("Sending Username ...", 0);
    if (send(this->sock, buffer.c_str(), buffer.size(), 0) == -1)
        throw std::runtime_error("Failed to send USER");
    sleep(1);
}

std::string Bot::PromtUser(std::string prompt)
{
    std::string buffer = "";
    while (buffer.empty())
    {
        std::cout << prompt;
        if (!std::getline(std::cin, buffer))
            throw (std::runtime_error("INPUT closed"));
        if (!buffer.empty())
            break;
        std::cout << "This field cannot be empty" << std::endl;
    }
    return (buffer);
}

void Bot::ParseAddress(std::string &server)
{
    if (server == "localhost")
        server = "127.0.0.1";
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sock == -1)
        throw std::runtime_error("Failed to create client socket");
    std::memset(&this->serv_addr, 0, sizeof(this->serv_addr));
    this->serv_addr.sin_family = AF_INET;
    this->serv_addr.sin_port = htons(this->port);
    if (inet_pton(AF_INET, server.c_str(), &serv_addr.sin_addr) <= 0)
    {
        close(this->sock);
        throw std::runtime_error("Invalid address/Address not supported");
    }
    if (connect(this->sock, (struct sockaddr *)&this->serv_addr, sizeof(this->serv_addr)) < 0)
    {
        close(this->sock);
        throw std::runtime_error("Connection Failed");
    }
}

Bot::Bot()
{

}

Bot::Bot(std::string server, std::string port)
{
    Status("Connecting...", 0);
    sleep(1);
    this->status = false;
    if (server.empty() || port.empty())
        throw std::runtime_error("The Server-ip/Port is empty !");
    this->sock = -1;
    this->port = Convert<int>(port);
    if (this->port < 1024 || this->port > 65535)
        throw std::runtime_error(std::string("[" + port + "]" + " Port is invalid !").c_str());
    this->ParseAddress(server);
    Status("Connected", 1);
}

Bot::~Bot()
{
    Status("Bot : bye!", 1);
    if (this->sock > 0)
        close (this->sock);
}

std::string Bot::GetTime()
{
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", now);
    return (std::string(buffer));
}

std::string Bot::GetDate()
{
    char buffer[80];
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
    return (std::string(buffer));
}

int Bot::ParseData(std::string buffer)
{
    int ret = 0;
    if (buffer.find("NOTICE") == std::string::npos)
        return (0);
    std::string sender = buffer.substr(buffer.find(':') + 1, buffer.find('!') - 1);
    std::string msj = buffer.substr(buffer.find_last_of(':') + 1);
    std::string result = "";
    /// this one for \r !!
    msj.pop_back();
    if (msj == "date")
    {
        result = "NOTICE " + sender + " :" + this->GetDate() + "\r\n";
        ret = 1;
    }
    else if (msj == "time")
    {
        result = "NOTICE " + sender + " :" + this->GetTime() + "\r\n";
        ret = 2;
    }
    else if (msj == "help")
    {
        result = "NOTICE " + sender + " : time,date" + "\r\n";
        ret = 3;
    }
    if (!result.empty())
        if (send(this->sock, result.c_str(), result.size(), 0) == -1)
            throw std::runtime_error("Failed to send back the message to client");
    return (ret);
}

void Bot::HandelBotReq()
{
    char buffer[1024] = {0};
    std::string actions[4] = {"ignore", "send date", "send time", "send help"};
    
    int n = recv(this->sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0)
    {
        std::string msj(buffer);
        msj.pop_back();
        if (!this->status)
        {
            if (msj.find("001") != std::string::npos)
            {
                this->status = true;
                Status("Bot now connected to server !, ready...", 1);
            }
            else
                throw std::runtime_error(msj.c_str());
        }
        else
        {
            Status(std::string("Bot recieve -> " + msj).c_str(), 0);
            Status(std::string("Bot action : " + actions[this->ParseData(msj)]).c_str(), 0);
        }
    }
    if (n == 0)
    {
        close(this->sock);
        throw std::runtime_error("Server disconnected");
    }
}

void SetNonBlockingFd(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("(fcntl) failed to get flag");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("(fcntl) failed to set flag");
}

void Bot::StartListen()
{
    SetNonBlockingFd(this->sock);

    struct pollfd tmp;
    tmp.fd = this->sock;
    tmp.events = POLLIN;

    while (true) 
    {
        int poll_count = poll(&tmp, 1, 0);
        if (poll_count == -1)
            throw std::runtime_error("Poll() fail??");
        if (tmp.revents & POLLIN)
            HandelBotReq();
    }
}