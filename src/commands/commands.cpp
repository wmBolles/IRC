#include "commands.hpp"

int isValidIrcCmd(const std::string toCheck)
{
    std::string arr[] = {"PASS", "NICK", "USER", "JOIN",
        "TOPIC", "INVITE", "KICK", "MODE", "PRIVMSG", "NOTICE"};
    std::vector<std::string> ircCmds(arr, arr + 10);
    std::vector<std::string>::iterator it = std::find(ircCmds.begin(), ircCmds.end(), toCheck);
    if (it != ircCmds.end())
        return (1);
    return (0);
}

// This function check in both key and value if an equal sign exists
// as equal sign is the separator by default for a tag; so too many
// equal signs can lead to a confusion.
int unexpectedEqChar(const std::string &token)
{
    size_t unexpEqPos = token.find_first_of('=');
    if (unexpEqPos != std::string::npos)
        return (-1);
    return 0;
}

// This function splits each tag to a pair key=value
int splitPair(const std::string &pair, std::string &key, std::string &value)
{
    size_t eqPos = pair.find('=');
    if (eqPos != std::string::npos)
    {
        key = pair.substr(0, eqPos);
        value = pair.substr(eqPos + 1);
    }
    else
        key = pair.substr();
    if (unexpectedEqChar(key) || unexpectedEqChar(value))
        return -1;
    return (0);
}

int processTags(msg & m, std::string token)
{
    std::string pair = "";
    std::string key = "";
    std::string value = "";

    while (true)
    {
        key.clear();
        value.clear();
        size_t pos = token.find(';');
        if (pos != std::string::npos)
        {
            pair = token.substr(0, pos);
            token.erase(0, pair.size() + 1); // Plus one to erase the comma ';'
            if (splitPair(pair, key, value))
                return (-1);
            if (!key.empty())
                m.tags[key] = value;
        }
        else
        {
            pair = token.substr();
            if (splitPair(pair, key, value))
                return (-1);
            if (!key.empty())
                m.tags[key] = value;
            break;
        }
    }
    return (0);
}

int parseMsg(Server &srv, std::vector<Client>::iterator client, msg &m, const std::string &msgRecv)
{
    std::stringstream ss(msgRecv);
    std::string token;
    std::string errMsg;

    if (msgRecv.empty())
        return (0);
    while (ss >> token)
    {

        if (token[0] == '@' && m.cmd.empty())
        {
            if (processTags(m, token.substr(1)))
            {
                errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " :Could not process because too many equal operator\r\n"; 
                srv.SendMsg(client->id, errMsg);
                return (-1);
            }
        }
        else if (token[0] == ':' && m.cmd.empty())
        {
            errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " :Could not process because source not allowed\r\n"; 
            srv.SendMsg(client->id, errMsg);
            return (-1);
        }
        else if (isValidIrcCmd(token))
        {
            if (!m.cmd.empty())
            {
                errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " :Could not process multiple commands\r\n";
                srv.SendMsg(client->id, errMsg);
                return (-1);
            }
            m.cmd = token;
        }
        else if (!m.cmd.empty() && token[0] != '@')
        {
            if (token[0] == ':')
            {
                std::string trailingParam = ss.str().substr(ss.str().find(':') + 1);
                m.params.push_back(trailingParam);
                break;
            } else
                m.params.push_back(token);
        }
        else
        {
            errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNCOMMAND) + " " + client->nickname + " :Could not process because invalid message format\r\n";
            srv.SendMsg(client->id, errMsg);
            return (-1);
        }
    }
    if (m.cmd.empty())
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " :Could not process because no command\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return (0);
}

void ProcessLine(Server &srv, std::vector<Client>::iterator client, std::string buffer)
{
    msg ircMsg;
    std::stringstream pongSS;

    if (buffer.empty())
        return;
    pongSS << buffer;
    if (pongSS.str().substr(0, pongSS.str().find('G') + 1) == "PONG")
        return ;
    if (parseMsg(srv, client, ircMsg, buffer))
    {
        Status(std::string("Can't process " + (*client).nickname + "'s message, syntax error!").c_str(), 0);
        return;
    }
    if (checkCmdParams(srv, client, ircMsg))
    {
        Status(std::string("Can't process " + (*client).nickname + "'s message, invalid parameters!").c_str(), 0);
        return;
    }
    if (process_cmds(srv, client, ircMsg))
    {
        Status(std::string("Can't process " + (*client).nickname + "'s req").c_str(), 0);
        return;
    }
    return;
}
