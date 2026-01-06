/* ============================================================= *
 * Project   : project                                           *
 * File      : ft_irc.hpp                                        *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#ifndef HEADER
#define HEADER

#include "Status.hpp"
#include "Server.hpp"
#include "Bot.hpp"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include <signal.h>


#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define RESET   "\033[0m"
#define SRVNAME "1337IrcServer"

// ERRORS
# define ERR_UNKNOWNERROR "400"
# define ERR_NOSUCHNICK "401"
# define ERR_NOSUCHCHANNEL "403"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_TOOMANYCHANNELS "405" 
# define ERR_WASNOSUCHNICK "406"
# define ERR_TOOMANYTARGETS "407"
# define ERR_NOTEXTTOSEND "412" 
# define ERR_INPUTTOOLONG "417" 
# define ERR_UNKNOWNCOMMAND "421" 
# define ERR_NONICKNAMEGIVEN "431" 
# define ERR_ERRONEUSNICKNAME "432" 
# define ERR_NICKNAMEINUSE "433" 
# define ERR_NICKCOLLISION "436"
# define ERR_USERNOTINCHANNEL "441" 
# define ERR_NOTONCHANNEL "442"
# define ERR_USERONCHANNEL "443" 
# define ERR_NOTREGISTERED "451" 
# define ERR_NEEDMOREPARAMS "461" 
# define ERR_ALREADYREGISTERED "462"
# define ERR_PASSWDMISMATCH "464"
# define ERR_YOUREBANNEDCREEP "465" 
# define ERR_CHANNELISFULL "471" 
# define ERR_UNKNOWNMODE "472" 
# define ERR_INVITEONLYCHAN "473" 
# define ERR_BANNEDFROMCHAN "474"
# define ERR_BADCHANNELKEY "475"
# define ERR_NOPRIVILEGES "481"
# define ERR_CHANOPRIVSNEEDED "482"
# define ERR_USERSDONTMATCH "502"
# define ERR_INVALIDKEY "525"
# define ERR_INVALIDMODEPARAM "696"
# define ERR_NOPRIVS "723"

// REPLY NUMERIC FLAGS
# define RPL_UMODEIS "221"
# define RPL_CHANNELMODEIS "324"
# define RPL_CREATIONTIME "329"
# define RPL_NOTOPIC "331"
# define RPL_TOPIC "332" 
# define RPL_TOPICWHOTIME "333"
# define RPL_INVITELIST "336"
# define RPL_ENDOFINVITELIST "337"
# define RPL_INVITING "341"
# define RPL_NAMREPLY "353"
# define RPL_ENDOFNAMES "366"

class Server;
struct Client;
struct Channel;

//  display help in std::err
const char  *DisplayHelp(int status);
// init the server!
void        ft_init(int ac, char **av);
// checkes for the port is only a digit
bool        OnlyNums(std::string str);
// this function for handling the req from client
// its will return a number to handel the action
// for example it will return 1 to disconnect the client
// 0 is the default, look at ft_init.cpp line 45
void        CloseServer(int sig);
void        SetRunStatus(int val);
bool        GetRunStatus();
std::string GetTime();
std::string GetDate();
std::string GetTimeAndDate();
void        SetNonBlockingFd(int fd);
void        HandelClient(Server &srv, int id);
void        ProcessLine(Server& srv, std::vector<Client>::iterator client, std::string buffer);

#endif