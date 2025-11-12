#include "commands.hpp"

int checkJoinCmdOneParam(Server & srv, msg & m,
    std::vector<Client>::iterator client,
    std::vector<std::string> & params)
{
    std::string errMsg;

    if (params[0].find_first_of(' ') != std::string::npos)
    {
        errMsg = ":" + std::string(SRVNAME) 
        + " " + std::string(ERR_UNKNOWNERROR) + " " 
        + client->nickname + " " + m.cmd + " :Unexpected token in the parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    while (true)
    {
        size_t commaPos = params[0].find(',');
        if (commaPos != std::string::npos)
        {
            if (params[0].substr(0, commaPos)[0] != '#')
            {
                errMsg = ":" + std::string(SRVNAME)  + " "
                + std::string(ERR_UNKNOWNERROR) + " " 
                + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
            size_t len = params[0].substr(0, commaPos).size();
            m.chanAndKeys[params[0].substr(0, commaPos)] = "";
            params[0].erase(0, len + 1);
        }
        else
        {
            if (params[0][0] != '#')
            {
                errMsg = ":" + std::string(SRVNAME)  + " " 
                + std::string(ERR_UNKNOWNERROR) + " " 
                + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
            m.chanAndKeys[params[0]] = "";
            break;
        }
    }
    return 0;
}

int checkJoinCmdParams(Server &srv, std::vector<Client>::iterator client, msg &m)
{
    std::string errMsg;
    std::vector<std::string> params(m.params);

    if (params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (params.size() > 2)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Too many parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (params.size() == 1)
    {
        if (checkJoinCmdOneParam(srv, m, client, params))
            return -1;
    }
    else
    {
        std::vector<std::string> channels;
        std::vector<std::string> keys;
        std::vector<std::string> params0;
        std::vector<std::string> params1;
        std::string chan;
        std::string key;

        params0.push_back(params[0]);
        params1.push_back(params[1]);
        while (true)
        {
            size_t commaPos = params0[0].find(',');
            if (commaPos != std::string::npos)
            {
                chan = params0[0].substr(0, commaPos);
                if (chan[0] != '#')
                {
                    errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
                    srv.SendMsg(client->id, errMsg);
                    return -1;
                }
                size_t len = chan.size();
                params0[0].erase(0, len + 1);
                channels.push_back(chan);
            }
            else
            {
                if (params0[0][0] != '#')
                {
                    errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
                    srv.SendMsg(client->id, errMsg);
                    return -1;
                }
                channels.push_back(params0[0]);
                break;
            }
        }
        while (true)
        {
            size_t commaPos = params1[0].find(',');
            if (commaPos != std::string::npos)
            {
                key = params1[0].substr(0, commaPos);
                if (key[0] == '#')
                {
                    errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Unexpected char leading channel key\r\n";
                    srv.SendMsg(client->id, errMsg);
                    return -1;
                }
                size_t len = key.size();
                params1[0].erase(0, len + 1);
                keys.push_back(key);
            }
            else
            {
                if (params1[0][0] == '#')
                {
                    errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Unexpected char leading channel key\r\n";
                    srv.SendMsg(client->id, errMsg);
                    return -1;
                }
                keys.push_back(params1[0]);
                break;
            }
        }
        if (channels.size() < keys.size())
        {
            errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Too many keys\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
        for (unsigned int i = 0; i < channels.size(); i++)
        {
            if (i < keys.size())
                m.chanAndKeys[channels[i]] = keys[i];
            else
                m.chanAndKeys[channels[i]] = "";
        }
    }
    return 0;
}

int checkKickCmdParams(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() != 2 && m.params.size() != 3)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0][0] != '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[1][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Expected a user name as second parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() == 3 && m.params[2][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Unexpected token leading the comment\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkUserCmdParams(Server &srv, std::vector<Client>::iterator client, msg &m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() != 4)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0] != "0" || m.params[1] != "*")
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Expected '0' and '*' as second and third parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[2][0] == '#' || m.params[2].find(' ') != std::string::npos
        || m.params[3][0] == '#' || m.params[3].find(' ') != std::string::npos)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Unexpected char in the parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkModeCmdParams(Server & srv, std::vector<Client>::iterator client,msg & m)
{
    std::string errMsg;
    std::string rplMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0][0] != '#' && m.params.size() > 1)
    {
        errMsg = ":" + std::string(SRVNAME)  + " "
            + std::string(ERR_UNKNOWNERROR) + " "
            + client->nickname + " " + m.cmd
            + " :Invalid channel name\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    for (unsigned int i = 0; i < m.params.size(); i++)
    {
        if (m.params[i].find(' ') != std::string::npos)
        {
            errMsg = ":" + std::string(SRVNAME) + " "
                + std::string(ERR_UNKNOWNERROR) + " "
                + client->nickname + " :Found space in parameter\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
    }
    if (m.params.size() == 2 && m.params[1].find('o') != std::string::npos)
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NEEDMOREPARAMS) + " "
            + client->nickname + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() > 1
        && m.params[1][0] != '+'
        && m.params[1][0] != '-')
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_UNKNOWNERROR) + " "
            + client->nickname + " :Invalid modestring\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() >= 2)
    {
        for (unsigned int i = 0; i < m.params.size(); i++)
        {
            if (m.params[i].find(' ') != std::string::npos)
            {
                errMsg = ":" + std::string(SRVNAME) + " "
                    + std::string(ERR_UNKNOWNERROR) + " "
                    + client->nickname + " :Unexpected token leading parameter\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
        }
        unsigned int i = 1;
        for (unsigned int j = 2; j < m.params.size();)
        {
            if (i < m.params[1].size())
            {
                if (m.params[1][i] != 'k'
                    && m.params[1][i] != 'o'
                    && m.params[1][i] != 'l')
                {
                    m.modesAndArgs[std::string(1, m.params[1][i])] = "";
                } else {
                    if (m.params[1][i] == 'o')
                    {
                        std::stringstream prefix;
                        prefix << i;
                        m.modesAndArgs[prefix.str() + std::string(1, m.params[1][i])] = m.params[j];
                        prefix.str("");
                    } else
                        m.modesAndArgs[std::string(1, m.params[1][i])] = m.params[j];
                    j++;
                }
            }
            i++;
        }
        for (; i < m.params[1].size(); i++)
            m.modesAndArgs[std::string(1, m.params[1][i])] = "";
    }
    std::map<std::string, std::string>::iterator it = m.modesAndArgs.begin();
    for (; it != m.modesAndArgs.end(); it++)
    {
        if (it->first[0] == 'k')
        {
            if (it->second.size() < 4 && m.params[1][0] == '+')
            {
                errMsg = ":" + std::string(SRVNAME) + " "
                    + std::string(ERR_INVALIDMODEPARAM) + " "
                    + client->nickname + " " + m.params[0] + " "
                    + it->first[0] + " :Key string too short\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
            if (it->second.find(' ') != std::string::npos
                || it->second[0] == '#')
            {
                errMsg = ":" + std::string(SRVNAME) + " "
                    + std::string(ERR_INVALIDKEY) + " "
                    + client->nickname + " " + m.params[0]
                    + " :Key is not well-formed\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
        }
        if (it->first[0] == 'l')
        {
            if (!it->second.empty() &&
                it->second.find_first_not_of("0123456789") != std::string::npos
                && m.params[1][0] == '+')
            {
                errMsg = ":" + std::string(SRVNAME) + " "
                    + std::string(ERR_UNKNOWNERROR) + " "
                    + client->nickname + " :The limit must be an integer\r\n";
                srv.SendMsg(client->id, errMsg);
                return -1;
            }
        }
    }
    return 0;
}

int checkPrivmsgCmdParams(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() != 2)
    {
        if (m.params.size() == 1 && m.params[0].find(' ') == std::string::npos)
        {
            errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NOTEXTTOSEND) + " " + client->nickname + " " + m.cmd + " :No text to send\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[1][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_TOOMANYTARGETS) + " " + client->nickname + " " + m.cmd + " :Too many targets\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

// This command is used by the bot
int checkNoticeCmdParams(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() != 2)
    {
        if (m.params.size() == 1 && m.params[0].find(' ') == std::string::npos)
        {
            errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NOTEXTTOSEND) + " " + client->nickname + " " + m.cmd + " :No text to send\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[1][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_TOOMANYTARGETS) + " " + client->nickname + " " + m.cmd + " :Too many targets\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkInviteCmdParams(Server &srv, std::vector<Client>::iterator client, msg &m)
{
    std::string errMsg;

    if (m.params.empty())
        return 0;
    if (m.params.size() == 1 || m.params.size() > 2)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :The first parameter can't be a channel name\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[1][0] != '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0].find(' ') != std::string::npos || m.params[1].find(' ') != std::string::npos)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Unexpected char in the parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkTopicCmdParams(Server &srv, std::vector<Client>::iterator client, msg &m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() > 2)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid number of parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0][0] != '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Invalid channel name\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() == 2 && m.params[1][0] == '#')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NEEDMOREPARAMS) + " " + client->nickname + " " + m.cmd + " :Too many channels\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkNickCmdParams(Server &srv, std::vector<Client>::iterator client, msg &m)
{
    std::string errMsg;

    if (m.params.empty())
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_NONICKNAMEGIVEN) + " " + client->nickname + " " + m.cmd + " :No nickname given\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params.size() > 1)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Too many parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0].find(' ') != std::string::npos)
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Unexpected char in the parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[0][0] == '#' || m.params[0][0] == ':')
    {
        errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_UNKNOWNERROR) + " " + client->nickname + " " + m.cmd + " :Invalid char leading parameter\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int checkCmdParams(Server &srv, std::vector<Client>::iterator client, msg &ircMsg)
{
    int error = 0;
    std::string errMsg;

std::string arr[] = {"PASS", "JOIN", "KICK", "USER",
                    "MODE", "PRIVMSG", "NOTICE", "INVITE", "TOPIC", "NICK"};
    std::vector<std::string> cmds(arr, arr + 10);
    unsigned int i = 0;
    for (; i < cmds.size(); i++)
    {
        if (ircMsg.cmd == cmds[i])
            break;
    }
    ircMsg.cmdIdx = i;
    switch (i)
    {
        case 0:
            errMsg = ":" + std::string(SRVNAME)  + " " + std::string(ERR_ALREADYREGISTERED) + " " + client->nickname + " :You are already registered\r\n";
            srv.SendMsg(client->id, errMsg);
            error = 1;
            break;
        case 1:
            if (checkJoinCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 2:
            if (checkKickCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 3:
            errMsg = ":" + std::string(SRVNAME)  + " "
                + std::string(ERR_ALREADYREGISTERED) + " "
                + client->nickname +  " :You are already registered\r\n";
            srv.SendMsg(client->id, errMsg);
            error = 1;
            break;
        case 4:
            if (checkModeCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 5:
            if (checkPrivmsgCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 6:
            break;
        case 7:
            if (checkInviteCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 8:
            if (checkTopicCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        case 9:
            if (checkNickCmdParams(srv, client, ircMsg))
                error = 1;
            break;
        default:
            break;
    }
    if (error)
        return -1;
    return (0);
}
