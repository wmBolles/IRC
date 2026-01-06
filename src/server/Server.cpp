/* ============================================================= *
 * Project   : project                                           *
 * File      : Server.cpp                                        *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#include "Server.hpp"
#include <cerrno>

Server::Server(std::string _port, std::string _password) : Password(_password)
{
    Status("Server starting...", 0);
    this->TotalClients = 0;
    this->TotalChannels = 0;
    this->CreationTime = GetTimeAndDate();
    Status("Creating server socket", 0);
    this->SockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->SockFd == -1)
        throw std::runtime_error("Failed to create server socket!");
    Status("Socket created successfully", 1);
    this->Port = this->Convert<int>(_port);
    if (this->Port < 1024 || this->Port > 65535)
        throw std::runtime_error(std::string("Invalid Port " + _port).c_str());
    this->add.sin_family = AF_INET;
    this->add.sin_addr.s_addr = INADDR_ANY;
    this->add.sin_port = htons(this->GetPort());
    int opt = 1;
    setsockopt(this->SockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    Status("Binding...", 0);
    if (bind(this->SockFd, (struct sockaddr *)&this->add, sizeof(this->add)) == -1)
        throw(std::runtime_error("Failed to bind port and ip with socket"));
    Status("Binded successfully", 1);
    if (listen(this->SockFd, SOMAXCONN) == -1)
        throw(std::runtime_error(std::string("Failed to Listen on " + _port).c_str()));
    Status(std::string("Listening on Port     : " + _port).c_str(), 1);  
    Status(std::string("  *********  Password : " + _password).c_str(), 1);
    Status("Server started successfully", 1);
}

Server::~Server()
{
    Status("Closing the server", 0);
    Status("Disconnecting clients", 0);
    for (size_t i = 0; i < this->pollfds.size(); i++)
        close (this->pollfds[i].fd);
    Status("Clients Disconnected", 1);
    Status("Server closed successfully", 1);
}

int Server::GetPort() const
{
    return (this->Port);
}

std::string Server::GetPass() const
{
    return (this->Password);
}

int Server::GetSockFd() const
{
    return (this->SockFd);
}

sockaddr_in *Server::GetServerAddress()
{
    return (&this->add);
}

bool Server::CheckName(std::string& name)
{
    if (this->clients.empty())
        return (1);
    std::vector<Client>::iterator i = this->clients.begin();
    for (; i != this->clients.end(); ++i)
        if ((*i).nickname == name)
            return (0);
    return (1);
}

int Server::AddClient(int fd, std::string username, std::string nickname)
{
    Client NewC;
    NewC.auth = -1;
    NewC.fd = fd;
    NewC.id = TotalClients;
    NewC.nickname = nickname;
    NewC.username = username;
    NewC.modes = "";
    NewC.buffer = "";
    this->clients.push_back(NewC);
    this->TotalClients++;
    return (NewC.id);
}

std::string Server::getPrefix(Client & client) const
{
    return ":" + client.nickname + "!"
            + (client.username.empty() ? client.nickname : client.nickname)
            + "@" + client.hostname;
}

void Server::removDisconnClientFromChan(int id)
{
    Client &cl = *GetClientById(id);

    for (size_t i = 0; i < channels.size(); i++)
    {
        std::vector<int>::iterator it_client = std::find(channels[i].clients.begin(), channels[i].clients.end(), id);
        std::vector<int>::iterator it_op = std::find(channels[i].operators.begin(), channels[i].operators.end(), id);

        if (it_client != channels[i].clients.end() || it_op != channels[i].operators.end())
            SendMsgToChannel(i, ":" + cl.nickname + "!" + cl.username + "@" + cl.hostname + " QUIT :Connection closed\r\n");
        if (it_client != channels[i].clients.end())
        {
            channels[i].clients.erase(it_client);
            channels[i].chanUsersNum--;
        }
        else if (it_op != channels[i].operators.end())
        {
            channels[i].operators.erase(it_op);
            channels[i].chanUsersNum--;
        }
    }
    cl.opChan.clear();
}


void Server::RmClient(int id)
{
    std::vector<Client>::iterator i = this->GetClientById(id);


    removDisconnClientFromChan(id);
    if (i == this->clients.end())
        throw std::runtime_error("Unable to find this client to remove");
    close((*i).fd);
    Status(std::string((*i).nickname + " Disconnected !").c_str(), 0);

    this->clients.erase(i);
}

std::vector<Client>::iterator Server::GetClientById(int id)
{
    if (this->clients.empty())
        throw std::runtime_error("No client yet");
    std::vector<Client>::iterator i = this->clients.begin();
    for (; i != this->clients.end(); ++i)
        if ((*i).id == id)
            break;
    if (i == this->clients.end())
        throw (std::runtime_error("this client id not found !"));
    return (i);
}

int Server::GetLastClientId()
{
    return (this->TotalClients - 1);
}

int Server::GetClientFdById(int id)
{
    return ((*GetClientById(id)).fd);
}

std::string Server::GetClientNameById(int id)
{
    return ((*GetClientById(id)).nickname);
}

std::vector<Client>::const_iterator Server::GetClientsLastIt() const 
{
    return this->clients.end();
}

int Server::GetClientIdByName(std::string name)
{
    std::vector<Client>::iterator i = clients.begin();
    for (; i != clients.end(); i++)
        if ((*i).nickname == name)
            break ;
    if (i == clients.end())
        throw (std::runtime_error(std::string("Enable to find this name id " + name)));
    return ((*i).id);
}

void    Server::SendMsg(int id, std::string msg)
{
    try 
    {
        std::vector<Client>::iterator i = GetClientById(id);
        send((*i).fd, msg.c_str(), msg.length(), 0);
    } 
    catch (const std::exception & e)
    {
        Status(std::string("Error : " + std::string(e.what()) + " or he left server").c_str(), 0);
    }
}

void    Server::SendMsgByFd(int fd, std::string msg)
{
    send(fd, msg.c_str(), msg.length(), 0);
}


void    Server::BroadCast(std::string msj)
{
    for (int i = 0; i < TotalClients; i++)
        SendMsg(i, msj);
}

/* channels topic */

Channel& Server::GetChannelByName(std::string name)
{
    if (channels.empty())
        throw std::runtime_error("No channels found");
    std::vector<Channel>::iterator i = channels.begin();

    for(; i != channels.end(); i++)
        if ((*i).name == name)
            break;
    if (i == channels.end())
        throw (std::runtime_error(std::string("enable to find this channel " + name).c_str()));
    return (*i);
}

Channel& Server::GetChannelById(int id)
{
    if (channels.empty())
        throw std::runtime_error("No channels found");
    std::vector<Channel>::iterator i = channels.begin();

    for(; i != channels.end(); i++)
        if ((*i).id == id)
            break;
    if (i == channels.end())
        throw (std::runtime_error("Enable to find channel by id"));
    return (*i);
}

std::vector<Channel>::iterator Server::GetChannelIterator(int id)
{
    if (channels.empty())
        return channels.end();
    std::vector<Channel>::iterator i = channels.begin();

    for(; i != channels.end(); i++)
        if ((*i).id == id)
            break;
    if (i == channels.end())
        return channels.end();
    return (i);
}

std::vector<Channel>::iterator Server::GetChannelIterator(std::string name)
{
    if (channels.empty())
        return channels.end();
    std::vector<Channel>::iterator i = channels.begin();

    for(; i != channels.end(); i++)
        if ((*i).name == name)
            break;
    if (i == channels.end())
        return channels.end();
    return (i);
}

std::vector<std::string> Server::getChanInvList(int clientId) const
{
    std::vector<std::string> chanList;

    for (unsigned int i = 0; i < this->channels.size(); i++)
    {
        for (unsigned int j = 0; j < channels[i].clientsInvited.size(); j++)
        {
            if (channels[i].clientsInvited[j] == clientId)
            {
                chanList.push_back(channels[i].name);
                break;
            }
        }
    }
    return chanList;
}

std::vector<Channel>::const_iterator Server::GetChannelLastIt() const
{
    return channels.end();
}

std::vector<int>::iterator Server::GetClientInsideChannel(int ChannelId, int ClientId)
{
    if (GetChannelById(ChannelId).clients.empty()
        && GetChannelById(ChannelId).operators.empty())
        throw (std::runtime_error("thier is no channel, using this id"));
    std::vector<int>::iterator i = GetChannelById(ChannelId).clients.begin();
    for (; i != GetChannelById(ChannelId).clients.end(); i++)
        if (*i == ClientId)
            return (i);
    std::vector<int>::iterator j = GetChannelById(ChannelId).operators.begin();
    for (; j != GetChannelById(ChannelId).operators.end(); j++)
        if (*j == ClientId)
            return (j);
    throw(std::runtime_error("failed to find this client"));
}


int     Server::AddAChannel(std::string name, std::string key, std::vector<Client>::iterator client)
{
    Channel NewChan;
    
    NewChan.type = 0;
    NewChan.id = TotalChannels;
    NewChan.invite_only = 0;
    NewChan.name = name;
    NewChan.key = key.empty() ? "" : key;
    NewChan.creationTime = time(NULL);
    NewChan.topic = "";
    NewChan.topicSetBy = client->nickname;
    NewChan.topicSetTime = time(NULL);
    NewChan.limit = -1;
    NewChan.operators.push_back(client->id);
    NewChan.chanUsersNum = 1;
    NewChan.modes.push_back("n");
    if (!NewChan.key.empty())
        NewChan.modes.push_back("k");
    TotalChannels++;
    this->channels.push_back(NewChan);
    return (NewChan.id);
}

void    Server::RmAChannel(int id)
{
    try
    {
        std::string name = GetChannelById(id).name;
        channels.erase(GetChannelIterator(id));
        Status(std::string("Channel " + name + " Deleted").c_str(), 1);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

int     Server::AddAClientToChannel(int ClientId, int ChannelId)
{
    std::string errMsg;

    GetChannelById(ChannelId).clients.push_back(ClientId);
    GetChannelById(ChannelId).chanUsersNum++;
    (*GetClientById(ClientId)).channel_id = ChannelId;
    Status(std::string("Adding " + GetClientNameById(ClientId) + " " + GetChannelById(ChannelId).name).c_str(), 1);
    return 0;
}

void     Server::RmAClientFromChannel(int ClientId, int ChannelId)
{
    GetChannelById(ChannelId).clients.erase(GetClientInsideChannel(ChannelId, ClientId));
    GetChannelById(ChannelId).chanUsersNum--;
}

void     Server::RmAnOpFromChannel(int ClientId, int ChannelId)
{
    GetChannelById(ChannelId).operators.erase(GetClientInsideChannel(ChannelId, ClientId));
    GetChannelById(ChannelId).chanUsersNum--;
}

void    Server::SendMsgToChannel(int ChannelId, std::string msg)
{
    Channel ch = GetChannelById(ChannelId);
    if (ch.clients.empty() && ch.operators.empty())
        throw (std::runtime_error("There is no client inside the channel"));
    std::vector<int>::iterator i = ch.clients.begin();
    for (; i != ch.clients.end(); ++i)
        SendMsg((*i), msg);
    std::vector<int>::iterator j = ch.operators.begin();
    for (; j != ch.operators.end(); ++j)
        SendMsg((*j), msg);
}

void    Server::SendMsgToChannel(int ChannelId, int issuerId, std::string msg)
{
    Channel ch = GetChannelById(ChannelId);
    if (ch.clients.empty() && ch.operators.empty())
        throw (std::runtime_error("There is no client inside the channel"));
    std::vector<int>::iterator i = ch.clients.begin();
    for (; i != ch.clients.end(); ++i)
    {
        if (*i != issuerId)
            SendMsg((*i), msg);
    }
    std::vector<int>::iterator j = ch.operators.begin();
    for (; j != ch.operators.end(); ++j)
    {
        if (*j != issuerId)
            SendMsg((*j), msg);
    }
}

bool    Server::IsClientInChannel(int ChannelId, int ClientId)
{
    if (GetChannelById(ChannelId).clients.empty() && GetChannelById(ChannelId).operators.empty())
        return (false);
    std::vector<int>::iterator i = GetChannelById(ChannelId).clients.begin();
    std::vector<int>::iterator end = GetChannelById(ChannelId).clients.end();
    for (; i != end; ++i)
        if (*i == ClientId)
            return (true);
    std::vector<int>::iterator opIt = GetChannelById(ChannelId).operators.begin();
    std::vector<int>::iterator opEnd = GetChannelById(ChannelId).operators.end();
    for (; opIt != opEnd; ++opIt)
        if (*opIt == ClientId)
            return true;
    return (false);
}

std::vector<Client>::iterator Server::GetClientByFd(int fd)
{
    if (clients.empty())
        throw (std::runtime_error("Empty list client"));
    std::vector<Client>::iterator i = this->clients.begin();
    for(; i != this->clients.end(); ++i)
        if ((*i).fd == fd)
            return (i);
    throw(std::runtime_error("No client found with this fd"));
}

void Server::SendAuthMessage(int id)
{
        SendMsg(id, ":" + std::string(SRVNAME) + " 001 " + GetClientNameById(id) + " :Welcome to " + std::string(SRVNAME) + " server!\r\n");
        SendMsg(id, ":" + std::string(SRVNAME) + " 002 " + GetClientNameById(id) + " :Your host is " + std::string(SRVNAME) + ", running version 1.0\r\n");
        SendMsg(id, ":" + std::string(SRVNAME) + " 003 " + GetClientNameById(id) + " :This server created in " + this->GetServerTime() + "\r\n");
        SendMsg(id, ":" + std::string(SRVNAME) + " 004 " + GetClientNameById(id) + " :" + std::string(SRVNAME) + " 1.0\r\n");
}

std::vector<struct pollfd>& Server::GetPollFds()
{
    return (this->pollfds);
}

std::string Server::GetServerTime() const
{
    return (this->CreationTime);
}

void Server::HandelNewConnection()
{
    struct sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t client_len = sizeof(client_addr);

    int new_socket = accept(this->SockFd, (sockaddr *)&client_addr, &client_len);
    if (new_socket == -1)
    {
        if (errno == EMFILE || errno == ENFILE)
        {
            Status("Fail to accept new Client: Too many open files (consider increasing ulimit)", 0);
        }
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            Status("Fail to accept new Client", 0);
        }
        return;
    }
    
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    SetNonBlockingFd(new_socket);
    (*this->GetClientById(AddClient(new_socket, "", ""))).hostname = client_ip;
    struct pollfd tmp;
    tmp.fd = new_socket;
    tmp.events = POLLIN;
    this->pollfds.push_back(tmp);
    Status("New Client Connection Accepted", 1);
}

void Server::HandleClientData(size_t idx)
{
    int fd = this->pollfds[idx].fd;
    std::vector<Client>::iterator i = this->GetClientByFd(fd);
    int id = i->id;
    char buffer[512] = {0};

    int n = recv(fd, buffer, 512, 0);
    if (n > 0)
    {
        if (std::string(buffer).find("\r\n") != std::string::npos)
        {
            try
            {
                (*i).buffer += buffer;
                HandelClient(*this, id);
            }
            catch (std::exception &e)
            {
            }
        }
        else
        {
            (*i).buffer += buffer;
            if ((*i).buffer.size() >= 510)
            {
                (*i).buffer += "\r\n";
                HandelClient(*this, id);
            }
        }
    }
    if (n <= 0)
    {
        this->pollfds.erase(this->pollfds.begin() + idx);
        this->RmClient(id);
    }
}

void Server::StartConnection()
{
    SetNonBlockingFd(this->GetSockFd());

    struct pollfd tmp;
    tmp.fd = this->SockFd;
    tmp.events = POLLIN;
    pollfds.push_back(tmp);

    while (GetRunStatus()) 
    {
        int poll_count = poll(pollfds.data(), pollfds.size(), 0);
        if (poll_count == -1)
            throw std::runtime_error("Poll() syscall fail??");
        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            struct pollfd pfd = pollfds[i];
            if (pfd.revents & POLLIN)
            {
                if (pfd.fd == this->SockFd)
                    this->HandelNewConnection();
                else
                    this->HandleClientData(i);
            }
        }
    }
}