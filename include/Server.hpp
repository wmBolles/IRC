/* ============================================================= *
 * Project   : project                                           *
 * File      : Server.hpp                                        *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#ifndef SERVER
#define SERVER

#include "ft_irc.hpp"
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

typedef struct Client
{
    int         auth;
    int         id;
    int         channel_id;
    std::string username;
    std::string nickname;
    std::string hostname;
    std::string buffer;
    std::string modes;
    int         fd;
    std::vector<int> opChan;
}   Client;

typedef struct Channel
{
    int                         type;
    int                         id;
    int                         chanUsersNum;
    int                         limit;
    int                         invite_only; 
    std::string                 name;
    std::string                 topic;
    std::string                 topicSetBy;
    time_t                      topicSetTime;
    time_t                      creationTime;
    std::vector<std::string>    modes;
    std::vector<int>            clients;
    std::vector<int>            operators;
    std::vector<int>            clientsBanned;
    std::vector<int>            clientsInvited;
    std::string                 key;
}   Channel;

class Server
{
    private :
        int                 TotalClients;
        int                 TotalChannels;
        sockaddr_in         add;
        int                 SockFd;
        int                 Port;
        std::string         Password;
        std::vector<Client> clients;
        std::vector<Channel> channels;
        std::string         CreationTime;
        std::vector<struct pollfd> pollfds;
    public :
        /*
        this topic is aboouut server
        */
        // this constractor start the server
        Server(std::string _port, std::string _password);

        // remove socket fd, and clients sockets fds
        ~Server();

        // get server port
        int GetPort() const;

        // get the server address struct
        sockaddr_in *GetServerAddress();

        // get password of server
        std::string GetPass() const;

        // get the server socket fd
        int         GetSockFd() const;

        // get server creation time
        std::string GetServerTime() const;

        // start the connection with client procc
        void StartConnection();
        void HandelNewConnection();
        void HandleClientData(size_t idx);

        /*
        this topic is aboouut clients
        */

        // this will return the id of the client
        // add a new  client to the list of clients, false if its already in list, true if not
        int        AddClient(int fd, std::string username, std::string nickname);

        // this fun will check if the name is already used
        bool        CheckName(std::string& name);

        // remove a client by id
        void        RmClient(int id);

        // get a client by id
        std::vector<Client>::iterator GetClientById(int id);

        // get last client id !
        int GetLastClientId();

        // get client fd by id
        int GetClientFdById(int id);

        // Get client prefix or source
        std::string getPrefix(Client & client) const;

        // get client name by id
        std::string GetClientNameById(int id);

        // get client by fd
        std::vector<Client>::iterator GetClientByFd(int fd);

        // get client id by name
        int         GetClientIdByName(std::string name);

        std::vector<Client>::const_iterator GetClientsLastIt() const;

        // send a message to a client by id
        void    SendMsg(int id, std::string msg);

        // send a message to client by FD
        void    SendMsgByFd(int fd, std::string msg);

        // send messages to all users that conncted
        void    BroadCast(std::string msj);

        // this only a function like atoi ...
        template <typename T>
        T Convert(std::string& val)
        {
            std::stringstream tmp(val);
            T ret;
            tmp >> ret;
            return (ret);
        }

        /*
        this topic is aboouut channels
        */

        // get a channel struct by name
       Channel& GetChannelByName(std::string name);

        // get a channel struct by id
       Channel& GetChannelById(int id);

       // get channel iterator inside channels of the server
       std::vector<Channel>::iterator GetChannelIterator(int id);

        // get a channel struct iterator inside channels vector by name
       std::vector<Channel>::iterator GetChannelIterator(std::string name);

       // This comes in handy when we want to check if an iterator
       // is not in the channels vector as  we can't access the private 
       // server channels members
       std::vector<Channel>::const_iterator GetChannelLastIt() const;

       // get client iterator inside server>channel>clients
       std::vector<int>::iterator GetClientInsideChannel(int ChannelId, int ClientId);

       // this function not checks for name or type its only adding (not safe)
       int      AddAChannel(std::string name, std::string key, std::vector<Client>::iterator client);

       // remove a channel
       void     RmAChannel(int id);

       // add a client to a channel
       int     AddAClientToChannel(int ClientId, int ChannelId);

       // remove client from channel
       void     RmAClientFromChannel(int ClientId, int ChannelId);

       // remove an operator form a channel
       void     RmAnOpFromChannel(int ClientId, int ChannelId);

        // send message to all members inside a channel
        void    SendMsgToChannel(int ChannelId, std::string msg);

        // Send a private msg to a channel excluding the issuer himself; if
        // he is a member
        void    SendMsgToChannel(int ChannelId, int issuerId, std::string msg);

        // check if the client is already in channel
        bool    IsClientInChannel(int ChannelId, int ClientId);
    
        // 
        void    removDisconnClientFromChan(int id);
        // This function return a list or a vector with each channel 
        // where the cleint is invited
        std::vector<std::string>    getChanInvList(int clientId) const;

        // this will send auth message
        void    SendAuthMessage(int id);

        // get pollfds vector,  to handel io!
        std::vector<struct pollfd>& GetPollFds();
};

#endif