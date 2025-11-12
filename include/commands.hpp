/* ============================================================= *
 * Project   : project                                           *
 * File      : commands.hpp                                      *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-29 11:28                                  *
 * License   : MIT                                               *
 * ============================================================= */

#ifndef COMMANDS_HPP
# define COMMANDS_HPP
// command header
# include <map>
#include "ft_irc.hpp"

// This struct represents an irc message. Below are the 
// differents parts or fields of this struct:
// - tags: optional parameter. If exists; it shoud start with at sign 
// symbol (@), tags is a comma-separated list if there are many tags.
// Each tag is a pair of <key=value>.
// - prefix or source: optional parameter. If exists, it should be preprnde 
// by a colon(:) and followed by a string, this string is the source; where 
// the message is coming from.
// - cmd: mandatory parameter. This is the action requested by the cleint.
// It should be a valid irc command (JOIN, KICK etc.)
// - params: mandatory if the command needs parameters. They are passed 
// to the command; and can be prepended by a colon(:), parameters are 
// separated by one or more spaces.
typedef struct
{
    std::map<std::string, std::string>  tags;
    std::string                         prefix; // Client don't have to add a source; only server is allowed.
    std::string                         cmd;
    int                                 cmdIdx; // This field is used in switch statement
    std::vector<std::string>            params;
    std::map<std::string, std::string>  chanAndKeys; // Holds channels and their keys
    std::map<std::string, std::string>  modesAndArgs; // Holds modes k and l and their arguments
} msg;

// The main function to process or parse the irc message. 
// Returns 0 if it succeeds and -1 in case of a failure.
int parseMsg(Server &srv, std::vector<Client>::iterator client, msg &m, const std::string &msgRecv);
int checkCmdParams(Server & srv, std::vector<Client>::iterator client, msg & ircMsg);
int process_cmds(Server & srv, std::vector<Client>::iterator client, msg & ircMsg);

#endif
