#include "commands.hpp"

// JOIN
int clientIsInvited(Channel & chan, int clientId)
{
    for (unsigned int i = 0; i < chan.clientsInvited.size(); i++)
    {
        if (chan.clientsInvited[i] == clientId)
            return 1;
    }
    return 0;
}

int clientIsBanned(Channel & chan, int clientId)
{
    for (unsigned int i = 0; i < chan.clientsBanned.size(); i++)
    {
        if (chan.clientsBanned[i] == clientId)
            return 1;
    }
    return 0;
}

std::string listOfCurrUsersInChan(Server & srv, Channel & chan)
{
    std::stringstream   listSs;

    for (unsigned int i = 0; i < chan.operators.size(); i++)
    {
        listSs << "@" + srv.GetClientById(chan.operators[i])->nickname;
        if (i < chan.operators.size() - 1)
            listSs << " ";
    }
    if (!chan.operators.empty() && !chan.clients.empty())
        listSs << " ";
    for (unsigned int i = 0; i < chan.clients.size(); i++)
    {
        listSs << srv.GetClientById(chan.clients[i])->nickname;
        if (i < chan.clients.size() - 1)
            listSs << " ";
    }
    return listSs.str();
}

void    srvReplyMsg(Server & srv, std::vector<Client>::iterator client, Channel & chan, msg & m)
{
    std::string srvReply;

    srvReply = srv.getPrefix(*client) + " " + m.cmd + " " + chan.name + "\n"
                +  ":" + std::string(SRVNAME) + " "
                + (!chan.topic.empty() ? std::string(RPL_TOPIC) : std::string(RPL_NOTOPIC))
                + " " + client->nickname + " "
                + chan.name + " :" + (!chan.topic.empty() ? chan.topic : "No topic is set") + "\n"
                + ":" + std::string(SRVNAME) + " " + std::string(RPL_NAMREPLY) + " "
                + client->nickname + " = " + chan.name + " :" + listOfCurrUsersInChan(srv, chan) + "\n"
                + ":" + std::string(SRVNAME) + " " + std::string(RPL_ENDOFNAMES) + " "
                + client->nickname + " " + chan.name + " :End of /NAMES list\r\n";
    srv.SendMsg(client->id, srvReply);
    srv.SendMsgToChannel(chan.id, srv.getPrefix(*client) + " " + m.cmd + " " + chan.name + "\r\n");
}

int notPassedLvlOneErrors(Server &srv, std::vector<Channel>::iterator itToFind, std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (srv.IsClientInChannel(itToFind->id, client->id))
    {
        errMsg = ":YarbiRihadLmra "
            + std::string(ERR_USERONCHANNEL) + " "
            + client->nickname + " :You are already on this channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (itToFind->invite_only && !clientIsInvited(*itToFind, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " 
            + std::string(ERR_INVITEONLYCHAN) + " "
            + client->nickname + " "
            + itToFind->name + " :Cannot join channel; invite-only mode(+i)\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int notPassedLvlTwoErrors(Server & srv, std::vector<Channel>::iterator itToFind, std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (clientIsBanned(*itToFind, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " 
            + std::string(ERR_BANNEDFROMCHAN) + " " 
            + client->nickname + " "
            + itToFind->name + " :Cannot join channel you are banned\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (itToFind->limit > -1 && itToFind->chanUsersNum == itToFind->limit)
    {
        errMsg = ":" + std::string(SRVNAME) + " " 
            + std::string(ERR_CHANNELISFULL) + " " 
            + client->nickname + " "
            + itToFind->name + " :Cannot join channel; channel is full(+l)\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int notPassedLvlThreeErrors(Server & srv, std::vector<Channel>::iterator itToFind,
    std::vector<Client>::iterator client, std::map<std::string, std::string>::iterator it)
{
    std::string errMsg;

    if (!itToFind->key.empty() && !clientIsInvited(*itToFind, client->id))
    {
        if (it->second.empty())
        {
            errMsg = ":" + std::string(SRVNAME) + " " 
                + std::string(ERR_BADCHANNELKEY) + " " 
                + client->nickname + " "
                + itToFind->name + " :Cannot join channel; need a key(password) (+k)\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
        if (it->second != itToFind->key)
        {
            errMsg = ":" + std::string(SRVNAME) + " " 
                + std::string(ERR_BADCHANNELKEY) + " " 
                + client->nickname + " "
                + itToFind->name + " :Cannot join channel; bad channel key(password) (+k)\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
    }
    return 0;
}

int process_join_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    int newChanId = -1;
    std::string errMsg;
    std::string srvReply;

    std::map<std::string, std::string>::iterator it = m.chanAndKeys.begin();
    for (; it != m.chanAndKeys.end(); it++)
    {
        std::vector<Channel>::iterator itToFind = srv.GetChannelIterator(it->first);
        if (itToFind == srv.GetChannelLastIt())
        {
            newChanId = srv.AddAChannel(it->first, it->second, client);
            // Add the operator mode to the client modes 
            client->modes += "o";
            client->opChan.push_back(newChanId);
            // srv.AddAChannel(it->first, it->second, 0, client);
            // We can add try-catch block as the function below throws 
            // an exception but here we are sure that this function cannnot.
            // If only the new channel was removed before we reach the 
            // statement below. 
            srvReplyMsg(srv, client, srv.GetChannelById(newChanId), m);
        } else {
            if (notPassedLvlOneErrors(srv, itToFind, client))
                return -1;
            if (notPassedLvlTwoErrors(srv, itToFind, client))
                return -1;
            if (notPassedLvlThreeErrors(srv, itToFind, client, it))
                return -1;
            if (srv.AddAClientToChannel(client->id, srv.GetChannelByName(it->first).id))
                return -1;
            if (clientIsInvited(srv.GetChannelByName(it->first), client->id))
                srv.GetChannelByName(it->first).clientsInvited.erase(
                        std::find(srv.GetChannelByName(it->first).clientsInvited.begin(),
                        srv.GetChannelByName(it->first).clientsInvited.end(),
                        client->id)
                );
            srvReplyMsg(srv, client, srv.GetChannelByName(itToFind->name), m);
        }
    }
    return 0;
}

// TOPIC
int viewChanTopic(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string rplMsg;

    if (srv.GetChannelIterator(m.params[0]) == srv.GetChannelLastIt())
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOSUCHCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :No such channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (!srv.IsClientInChannel(srv.GetChannelByName(m.params[0]).id, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOTONCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :You're not on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    rplMsg = ":" + std::string(SRVNAME) + " "
        + (srv.GetChannelByName(m.params[0]).topic.empty() ? std::string(RPL_NOTOPIC) : std::string(RPL_TOPIC))
        + " " + client->nickname + " " + m.params[0] + " :"
        + (srv.GetChannelByName(m.params[0]).topic.empty() ? "No topic is set" : srv.GetChannelByName(m.params[0]).topic)
        + "\n" + ":" + std::string(SRVNAME) + " "
        + std::string(RPL_TOPICWHOTIME)
        + " " + client->nickname + " " + m.params[0] + " "
        + srv.GetChannelByName(m.params[0]).topicSetBy + " "
        + std::to_string(srv.GetChannelByName(m.params[0]).topicSetTime) + "\r\n";
    srv.SendMsg(client->id, rplMsg);
    return 0;
}

// Checks if the channel is in the protected topic mode
// If it is only chanops can set the topic
int chanInProtTopicMode(Channel & chan)
{
    for (unsigned int i = 0; i < chan.modes.size(); i++)
    {
        if (chan.modes[i] == "t")
            return 1;
    }
    return 0;
}

// Checks if the Client is a chanop
int isChanOp(Channel & chan, int clientId)
{
    for (unsigned int i = 0; i < chan.operators.size(); i++)
    {
        if (chan.operators[i] == clientId)
            return 1;
    }
    return 0;
}

int clearChanTopic(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string rplMsg;

    if (srv.GetChannelIterator(m.params[0]) == srv.GetChannelLastIt())
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOSUCHCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :No such channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (!srv.IsClientInChannel(srv.GetChannelByName(m.params[0]).id, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOTONCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :You're not on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (chanInProtTopicMode(srv.GetChannelByName(m.params[0]))
        && !isChanOp(srv.GetChannelByName(m.params[0]), client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_CHANOPRIVSNEEDED) + " "
            + client->nickname  + " " + m.params[0] + " :You're not channel operator\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    srv.GetChannelByName(m.params[0]).topic = "";
    srv.GetChannelByName(m.params[0]).topicSetBy = client->nickname;
    std::cout << "Time and date: " << GetTimeAndDate() << std::endl;
    srv.GetChannelByName(m.params[0]).topicSetTime = time(NULL);
    rplMsg = srv.getPrefix(*client) + " " + m.cmd + " "
        + m.params[0] + " " +  " :" + srv.GetChannelByName(m.params[0]).topic
        + "\r\n";
    srv.SendMsgToChannel(srv.GetChannelByName(m.params[0]).id, rplMsg);
    return 0;
}

int setTopic(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string rplMsg;

    if (srv.GetChannelIterator(m.params[0]) == srv.GetChannelLastIt())
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOSUCHCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :No such channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (!srv.IsClientInChannel(srv.GetChannelByName(m.params[0]).id, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOTONCHANNEL) + " "
            + client->nickname  + " " + m.params[0] + " :You're not on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (chanInProtTopicMode(srv.GetChannelByName(m.params[0]))
        && !isChanOp(srv.GetChannelByName(m.params[0]), client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_CHANOPRIVSNEEDED) + " "
            + client->nickname  + " " + m.params[0] + " :You're not channel operator\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    srv.GetChannelByName(m.params[0]).topic = m.params[1];
    srv.GetChannelByName(m.params[0]).topicSetBy = client->nickname;
    srv.GetChannelByName(m.params[0]).topicSetTime = time(NULL);
    rplMsg = srv.getPrefix(*client) + " " + m.cmd + " "
        + m.params[0] + " :" + srv.GetChannelByName(m.params[0]).topic
        + "\r\n";
    srv.SendMsgToChannel(srv.GetChannelByName(m.params[0]).id, rplMsg);
    return 0;
}

int process_topic_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string rplMsg;
    std::string errMsg;
    
    // If TOPIC cmd received only one parameter(a channel)
    // show the topic of the channel only if the client
    // is part of that channel
    if (m.params.size() == 1)
    {
        if (viewChanTopic(srv, client, m))
            return -1;
        return 0;
    }
    if (m.params.size() == 2)
    {
        // If the topic string is empty; emptied the current topic
        // after checking that the if the t mode is set and that 
        // the client has privileges to set the topic.
        // Status(("Params size: " + std::string(m.params.size())).c_str(), 0);
        // Fix this when the topic is empty.
        if (m.params[1].empty() || m.params[1] == ":")
        {
            if (clearChanTopic(srv, client, m))
                return -1;
        } else 
        {
            if (setTopic(srv, client, m))
                return -1;
        }
    }
    return 0;
}

// PRIVMSG 
// This function processes the PRIVMSG command
// This function checks if a channel accepts external messages
// By default no as the +n mode is set.
int chanAcceptedExternalMsg(Channel & chan)
{
    for (unsigned int i = 0; i < chan.modes.size(); i++)
    {
        if (chan.modes[i] == "n")
            return 0;
    }
    return 1;
}

// Just check sif a client is in a given channel
int clientIsInChan(Channel & chan, int clientId)
{
    for (unsigned int i = 0; i < chan.clients.size(); i++)
    {
        if (chan.clients[i] == clientId)
            return 1;
    }
     for (unsigned int j = 0; j < chan.operators.size(); j++)
    {
        if (chan.operators[j] == clientId)
            return 1;
    }
    return 0;
}

// Sends a msg to a specific client
void sendPrvMsgToClient(Server & srv, std::vector<Client>::iterator client,
    msg & m, std::string & target, std::string & textToSend)
{
    std::string msgToSend;
    std::string errMsg;

    try {
        int targetId = srv.GetClientIdByName(target);
        msgToSend = ":" + client->nickname + " " + m.cmd
            + " " + target + " :" + textToSend + "\r\n";
        srv.SendMsg(targetId, msgToSend);
    } catch (...) {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NOSUCHNICK) + " "
            + client->nickname + " " + target + " "
            + " :No such nick\r\n";
        srv.SendMsg(client->id, errMsg);
    }
}

// This function is used to send a private message 
// inside a channel which not accept external messages
// only from its members
void    sendPrivMsgToChanNoExtMsg(Server & srv, std::vector<Client>::iterator client,
    msg & m, std::string & target, std::string & textToSend)
{
    std::string msgToSend;
    std::string errMsg;

    try {
        srv.GetClientInsideChannel(
            srv.GetChannelByName(target).id,
            client->id);
        msgToSend = ":" + client->nickname + " " + m.cmd
            + " " + target + " :" + textToSend + "\r\n";
        srv.SendMsgToChannel(
            srv.GetChannelByName(target).id,
            client->id, msgToSend);
    } catch (...) {
        errMsg = ":" +  std::string(SRVNAME)
            + " " + std::string(ERR_CANNOTSENDTOCHAN)
            + " " + client->nickname + " " + target
            + " :Cannot send to channel\r\n";
        srv.SendMsg(client->id, errMsg);
    }
}

void    sendPrivMsgToChanExtMsg(Server & srv, std::vector<Client>::iterator client,
    msg & m, std::string & target, std::string & textToSend)
{
    std::string msgToSend;

    msgToSend = ":" + client->nickname + " " + m.cmd
        + " " + target + " :" + textToSend + "\r\n";
    if (clientIsInChan(srv.GetChannelByName(target), client->id))
        srv.SendMsgToChannel(srv.GetChannelByName(target).id,
            client->id, msgToSend);
    else
        srv.SendMsgToChannel(
            srv.GetChannelByName(target).id, msgToSend);
}

int process_privmsg_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string param;
    std::string textToSend;

    param = m.params[0];
    if (m.params[1][m.params[1].size() - 1] == '\n')
        textToSend = m.params[1].substr(0, m.params[1].size() - 1);
    else 
        textToSend = m.params[1];
    while (!param.empty())
    {
        std::string target;
        size_t pos = param.find(',');
        if (pos != std::string::npos) {
            target = param.substr(0, pos);
            param.erase(0, pos + 1);
        } else
        {
            target = param;
            param.clear();
        }
        if (target[0] != '#')
            sendPrvMsgToClient(srv, client, m, target, textToSend);
        else {
            try {
                srv.GetChannelByName(target);
            } catch (...) {
                errMsg = ":" + std::string(SRVNAME)
                    + " " + std::string(ERR_NOSUCHCHANNEL)
                    + " " + client->nickname + " " + target
                    + " :No such channel\r\n";
                srv.SendMsg(client->id, errMsg);
                continue;
            }
            if (!chanAcceptedExternalMsg(srv.GetChannelByName(target)))
                sendPrivMsgToChanNoExtMsg(srv, client, m, target, textToSend);
            else
                sendPrivMsgToChanExtMsg(srv, client, m, target, textToSend);
        }
    }
    return 0;
}

// MODE
std::string getChanModes(Channel &chan)
{
    std::string modes;

    modes = "";
    for (unsigned int i = 0; i < chan.modes.size(); i++)
        modes += chan.modes[i];
    return modes;
}

void    showTargetModes(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string rplMsg;

    if (m.params[0][0] != '#')
    {
        rplMsg = ":" + std::string(SRVNAME) + " " 
            + std::string(RPL_UMODEIS) + " " + client->nickname 
            + " +" + client->modes + "\r\n";
        srv.SendMsg(client->id, rplMsg);
    } else {
        rplMsg = ":" + std::string(SRVNAME) + " "
            + std::string(RPL_CHANNELMODEIS) + " "
            + client->nickname + " " + m.params[0]
            + " +" + getChanModes(srv.GetChannelByName(m.params[0])) + "\n"
            + ":" + std::string(SRVNAME) + " " + std::string(RPL_CREATIONTIME)
            + " " + client->nickname + " " + m.params[0] + 
            + " " + std::to_string(srv.GetChannelByName(m.params[0]).creationTime)
            + "\r\n";
        srv.SendMsg(client->id, rplMsg);
    }
}

int isTargetValid(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;

    try {
        srv.GetClientById(srv.GetClientIdByName(m.params[0]));
    } catch (const std::exception & e)
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NOSUCHNICK) + " " + client->nickname
            + " " + m.params[0] + " :No such nick/channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (client->nickname != m.params[0])
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_USERSDONTMATCH)
            + " " + client->nickname + " :Cant change mode for other users\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int isValideModeChar(char c)
{
    if (c != 'i' && c != 't' && c != 'k' && c != 's'
        && c != 'o' && c != 'l' && c != 'n')
        return 0;
    return 1;
}

int modeCharExisted(Channel & chan, char cmode)
{
    for (unsigned int i = 0 ; i < chan.modes.size(); i++)
    {
        if (chan.modes[i][0] == cmode)
            return 1;
    }
    return 0;
}

int modeCharExisted(Client & client, char cmode)
{
    size_t pos = client.modes.find(cmode);
    if (pos != std::string::npos)
        return 1;
    return 0;
}

int clientIsChanOp(Client & client, int chanId)
{
    for (unsigned int i = 0; i < client.opChan.size(); i++)
    {
        if (client.opChan[i] == chanId)
            return 1;
    }
    return 0;
}

int     notPassedOperModeArgsErr(Server & srv, std::vector<Client>::iterator client, 
    std::map<std::string, std::string>::iterator it)
{
    std::string errMsg;

    if (it->second.empty())
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NEEDMOREPARAMS) + " "
            + client->nickname + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (it->second == client->nickname)
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_UNKNOWNERROR) + " "
            + client->nickname + " :Cannot applied a mode on yourseld\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

void    giveOperModeToClient(Server & srv, msg & m,
    std::vector<Client>::iterator clientIt,
    std::map<std::string, std::string>::iterator it, int id)
{
    if (!modeCharExisted(*clientIt, it->first[1]))
        clientIt->modes += it->first[1];
    if (!isChanOp(srv.GetChannelByName(m.params[0]), id))
    {
        srv.GetChannelByName(m.params[0]).operators.push_back(id);
        // When a client is given the chanop privileges 
        // We removed him from normal clients
        srv.GetChannelByName(m.params[0]).clients.erase(
            std::find(srv.GetChannelByName(m.params[0]).clients.begin(),
                    srv.GetChannelByName(m.params[0]).clients.end(),
                    id));
        clientIt->opChan.push_back(srv.GetChannelByName(m.params[0]).id);
    }
}

void    removOperModetoClient(Server & srv, msg & m,
    std::vector<Client>::iterator clientIt,
    std::map<std::string, std::string>::iterator it, int id)
{
    std::vector<int>::iterator clientToErase = std::find(
        srv.GetChannelByName(m.params[0]).operators.begin(),
        srv.GetChannelByName(m.params[0]).operators.end(), id);
    if (clientToErase != srv.GetChannelByName(m.params[0]).operators.end())
        srv.GetChannelByName(m.params[0]).operators.erase(clientToErase);
    // Here when a client is no longer a chanop after removing him
    // from chanops; we need to put him in normal clients
    srv.GetChannelByName(m.params[0]).clients.push_back(*clientToErase);
    std::vector<int>::iterator chanToErase = std::find(
        clientIt->opChan.begin(), clientIt->opChan.end(),
        srv.GetChannelByName(m.params[0]).id);
    if (chanToErase != clientIt->opChan.end())
        clientIt->opChan.erase(chanToErase);
    if (modeCharExisted(*clientIt, it->first[1]) && clientIt->opChan.empty())
        clientIt->modes.erase(clientIt->modes.find(it->first[1]), 1);
}

void    applyOperModeToClient(Server & srv, msg & m, std::vector<Client>::iterator client,
    std::map<std::string, std::string>::iterator it)
{
    std::string errMsg;

    try {
            int id = srv.GetClientIdByName(it->second);
            srv.GetClientInsideChannel(
            srv.GetChannelByName(m.params[0]).id, id);
            std::vector<Client>::iterator clientIt = srv.GetClientById(id);
            if (m.params[1][0] == '+')
                giveOperModeToClient(srv, m, clientIt, it, id);
            else
                removOperModetoClient(srv, m, clientIt, it, id);      
    } catch (...) {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_USERNOTINCHANNEL) + " "
            + client->nickname + " :User isn't on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
    }
}

void    applyInvOnlyModeToChan(Server & srv, msg & m,
    std::map<std::string, std::string>::iterator it)
{
    if (m.params[1][0] == '+' && !srv.GetChannelByName(m.params[0]).invite_only)
    {
        srv.GetChannelByName(m.params[0]).invite_only = 1;
        if (!modeCharExisted(srv.GetChannelByName(m.params[0]), 'i'))
            srv.GetChannelByName(m.params[0]).modes.push_back(it->first);
    }
    if (m.params[1][0] == '-'
         && srv.GetChannelByName(m.params[0]).invite_only)
    {
        srv.GetChannelByName(m.params[0]).invite_only = 0;
        if (modeCharExisted(srv.GetChannelByName(m.params[0]), 'i'))
            srv.GetChannelByName(m.params[0]).modes.erase(std::find(
                srv.GetChannelByName(m.params[0]).modes.begin(),
                srv.GetChannelByName(m.params[0]).modes.end(),
                it->first));
    }
}

void    applyExtMsgModetoChan(Server & srv, msg & m, 
    std::map<std::string, std::string>::iterator it)
{
    if (m.params[1][0] == '+')
    {
        if (!modeCharExisted(srv.GetChannelByName(m.params[0]), 'n'))
            srv.GetChannelByName(m.params[0]).modes.push_back(it->first);
    } else {
        if (modeCharExisted(srv.GetChannelByName(m.params[0]), 'n'))
            srv.GetChannelByName(m.params[0]).modes.erase(
                std::find(
                    srv.GetChannelByName(m.params[0]).modes.begin(),
                    srv.GetChannelByName(m.params[0]).modes.end(),
                    it->first));
    }
}

void    applyTopicModeToChan(Server & srv, msg & m,
    std::map<std::string, std::string>::iterator it)
{
    if (m.params[1][0] == '+')
    {
        if (!modeCharExisted(srv.GetChannelByName(m.params[0]), 't'))
            srv.GetChannelByName(m.params[0]).modes.push_back(it->first);
    }
    else
    {
        if (modeCharExisted(srv.GetChannelByName(m.params[0]), 't'))
            srv.GetChannelByName(m.params[0]).modes.erase(std::find(
                srv.GetChannelByName(m.params[0]).modes.begin(),
                srv.GetChannelByName(m.params[0]).modes.end(),
                it->first));
    }
}

void    applyKeyModeToChan(Server & srv, msg & m,
    std::vector<Client>::iterator client, 
    std::map<std::string, std::string>::iterator it,
    std::string & modesApplied)
{
    std::string errMsg;

    if (it->second.empty() && m.params[1][0] == '+')
    {
        std::cout << "Entre ici key" << std::endl;
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NEEDMOREPARAMS) + " "
            + client->nickname + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
    }
    modesApplied += it->first[0];
    if (m.params[1][0] == '+')
    {
        srv.GetChannelByName(m.params[0]).key = it->second;
        if (!modeCharExisted(srv.GetChannelByName(m.params[0]), 'k'))
            srv.GetChannelByName(m.params[0]).modes.push_back(it->first);
    }
    else
    {
        srv.GetChannelByName(m.params[0]).key = "";
        if (modeCharExisted(srv.GetChannelByName(m.params[0]), 'k'))
            srv.GetChannelByName(m.params[0]).modes.erase(std::find(
                srv.GetChannelByName(m.params[0]).modes.begin(),
                srv.GetChannelByName(m.params[0]).modes.end(),
                it->first));
    }
}

void    applyLimitModeToChan(Server & srv, msg & m,
    std::vector<Client>::iterator client,
    std::map<std::string, std::string>::iterator it, 
    std::string & modesApplied)
{
    std::string errMsg;

    if (it->second.empty() && m.params[1][0] == '+')
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NEEDMOREPARAMS) + " "
            + client->nickname + " :Not enough parameters\r\n";
        srv.SendMsg(client->id, errMsg);
    }
    modesApplied += it->first[0];
    if (m.params[1][0] == '+')
    {
        std::stringstream sslimit;
        sslimit << it->second;
        sslimit >> srv.GetChannelByName(m.params[0]).limit;
        if (!modeCharExisted(srv.GetChannelByName(m.params[0]), 'l'))
            srv.GetChannelByName(m.params[0]).modes.push_back(it->first);
        sslimit.str("");
        sslimit.clear();
    }
    else
    {
        srv.GetChannelByName(m.params[0]).limit = -1;
        if (modeCharExisted(srv.GetChannelByName(m.params[0]), 'l'))
            srv.GetChannelByName(m.params[0]).modes.erase(std::find(
                srv.GetChannelByName(m.params[0]).modes.begin(),
                srv.GetChannelByName(m.params[0]).modes.end(),
                it->first));
    }
}

void    broadcastModesApplied(Server & srv, msg & m,
    std::vector<Client>::iterator client,
    std::string & modesApplied)
{
    std::string rplMsg;
    std::string modesArgs = "";
    std::map<std::string, std::string>::iterator mArgsIt = m.modesAndArgs.begin();
    
    for (; mArgsIt != m.modesAndArgs.end(); )
    {
        if (!mArgsIt->second.empty())
            modesArgs.append(mArgsIt->second);
        mArgsIt++;
        if (mArgsIt != m.modesAndArgs.end() && !modesArgs.empty())
            modesArgs += ' ';
    }
    rplMsg = srv.getPrefix(*client) + " " + m.cmd + " "
        + m.params[0] + " " + modesApplied + " " + modesArgs
        + "\r\n";
    srv.SendMsgToChannel(srv.GetChannelByName(m.params[0]).id, rplMsg);
}

int notPassModeLvlOneErr(Server & srv,
    msg & m, std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (m.params[0][0] != '#')
    {
        if (isTargetValid(srv, client, m))
            return -1;
    } else 
    {
        try {
            srv.GetChannelByName(m.params[0]);
        } catch (const std::exception & e)
        {
            errMsg = ":" + std::string(SRVNAME) + " "
                + std::string(ERR_NOSUCHCHANNEL) + " "
                + client->nickname + " " + m.params[0]
                + " :No such channel\r\n";
            srv.SendMsg(client->id, errMsg);
            return -1;
        }
    }
    return 0;
}

int notPassModeLvlTwoErr(Server & srv, msg & m,
    std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (!isChanOp(srv.GetChannelByName(m.params[0]), client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_CHANOPRIVSNEEDED) + " "
            + client->nickname + " " + m.params[0]
            + " :You're not channel operator\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int process_mode_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string rplMsg;
    std::string errMsg;
    std::string modesApplied;

    if (notPassModeLvlOneErr(srv, m, client))
        return -1;
    if (m.params.size() == 1)
    {
        showTargetModes(srv, client, m);
        return 0;
    }
    if (notPassModeLvlTwoErr(srv, m, client))
        return -1;
    if (m.params[1][0] == '+')
        modesApplied = "+";
    else
        modesApplied = "-";
    std::map<std::string, std::string>::iterator it = m.modesAndArgs.begin();
    for (; it != m.modesAndArgs.end(); it++)
    {
        if (it->first.size() == 1 && !isValideModeChar(it->first[0]))
        {
            errMsg = ":" + std::string(SRVNAME) + " "
                + std::string(ERR_UNKNOWNMODE) + " "
                + client->nickname + " :is unknown mode char to me\r\n";
            srv.SendMsg(client->id, errMsg);
            continue;
        }
        if (it->first.size() > 1)
        {
            modesApplied += it->first[1];
            if (notPassedOperModeArgsErr(srv, client, it))
                continue;
            applyOperModeToClient(srv, m, client, it);
        } else {
            if (it->first[0] == 'i') {
                modesApplied += it->first[0];
                applyInvOnlyModeToChan(srv, m, it);
            }
            else if (it->first[0] == 'n') {
                modesApplied += it->first[0];
                applyExtMsgModetoChan(srv, m, it);
            }
            else if (it->first[0] == 't') {
                modesApplied += it->first[0];
                applyTopicModeToChan(srv, m, it);
            }
            else if (it->first[0] == 'k') {
                applyKeyModeToChan(srv, m, client, it, modesApplied);
            } else if (it->first[0] == 'l') {
                applyLimitModeToChan(srv, m, client, it, modesApplied);
            }
        }
    }
    broadcastModesApplied(srv, m, client, modesApplied);
    return 0;
}

// NOTICE
// This function handles the NOTICE cmd; 
// No reply to this command this can lead to an infinite responses
// between bot and the server or between bot and normal clients
int process_notice_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    if (m.params.size() != 2
        || m.params[0].find(' ') != std::string::npos
        || m.params[1][0] == '#')
        return 0;
    std::string target;
    std::string param = m.params[0];
    std::string msgToSend;
    while (!param.empty())
    {
        size_t pos = param.find(',');
        if (pos != std::string::npos)
        {
            target = param.substr(0, pos);
            param.erase(0, target.size() + 1);
        } else 
        {
            target = param.substr();
            param.erase(0);
        }
        std::string textToSend;
        if (m.params[1][m.params[1].size() - 1] == '\n')
            textToSend = m.params[1].substr(0, m.params[1].size() - 1);
        else
            textToSend = m.params[1];
        msgToSend = srv.getPrefix(*client) + " " + m.cmd
            + " " + target + " :" + textToSend + "\r\n";
        if (target[0] != '#')
        {
            try {
                srv.SendMsg(srv.GetClientIdByName(target), msgToSend);
            }
            catch (...) {}
        } else 
        {
            try {
                srv.SendMsgToChannel(srv.GetChannelByName(target).id, msgToSend);
            }
            catch(...) {}
            continue;
        }
    }
    return 0;
}

// INVITE 
void    rplInviteCmdMsg(Server &srv, std::vector<Client>::iterator client, msg & m)
{
    std::string rplMsgIssuer;
    std::string rplMsgTargetUser;

    rplMsgIssuer = ":" + std::string(SRVNAME) + " "
        + std::string(RPL_INVITING) + " " + client->nickname
        + " " + m.params[0] + " " + m.params[1] + "\r\n";
    srv.SendMsg(client->id, rplMsgIssuer);
    rplMsgTargetUser = srv.getPrefix(*client) + " " + m.cmd + " "
        + m.params[0] + " :" + m.params[1] + "\r\n";
    srv.SendMsg(srv.GetClientIdByName(m.params[0]), rplMsgTargetUser);
}

void    rplToInvCmdWithNoParam(Server & srv, std::vector<Client>::iterator client)
{
    std::string rplMsg = "";
    std::vector<std::string> chanInv;

    chanInv = srv.getChanInvList(client->id);
    for (unsigned int i = 0; i < chanInv.size(); i++)
    {
        rplMsg += ":" + std::string(SRVNAME) + " "
            + std::string(RPL_INVITELIST) + " "
            + client->nickname  + " " + chanInv[i]
            + " :You are invited\n";
    }
    rplMsg += ":" + std::string(SRVNAME) + " "
        + std::string(RPL_ENDOFINVITELIST) + " "
        + client->nickname + " :End of /INVITE list\r\n";
    srv.SendMsg(client->id, rplMsg);
}

int notPassedInvCmdLvlOneErr(Server & srv, msg & m, 
    std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (srv.GetChannelIterator(m.params[1]) == srv.GetChannelLastIt())
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NOSUCHCHANNEL) + " "
            + client->nickname + " " + m.params[1]
            + " :No such channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (!srv.IsClientInChannel(srv.GetChannelByName(m.params[1]).id, client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NOTONCHANNEL) + " "
            + client->nickname + m.params[1] + " :You're not on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    try {
        srv.GetClientIdByName(m.params[0]);
    } catch (const std::exception & e)
    {
        errMsg = ":" + std::string(SRVNAME) + " " 
            + std::string(ERR_NOSUCHNICK) + " "
            + client->nickname + " " + m.params[0] + " :No such nickname\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int notPassedInvCmdLvlTwoErr(Server & srv, msg & m,
    std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (srv.IsClientInChannel(srv.GetChannelByName(m.params[1]).id, 
        srv.GetClientIdByName(m.params[0])))
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_USERONCHANNEL) + " "
            + client->nickname + " "
            + m.params[0] + " " + m.params[1]
            + " :is already on channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (srv.GetChannelByName(m.params[1]).invite_only
        && !isChanOp(srv.GetChannelByName(m.params[1]), client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_CHANOPRIVSNEEDED) + " " + client->nickname
            + " " + m.params[1] + " :You're not channel operator\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int process_invite_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    // std::string rplMsgIssuer;
    // std::string rplMsgTargetUser;

    // When INVITE cmd is sent by the cleint with no parameter
    // the server must reply with a list of channels 
    // where the client was invited.
    if (m.params.empty())
    {
        rplToInvCmdWithNoParam(srv, client);
        return 0;
    }
    if (notPassedInvCmdLvlOneErr(srv, m, client))
        return -1;
    if (notPassedInvCmdLvlTwoErr(srv, m, client))
        return -1;
    if (!clientIsInvited(srv.GetChannelByName(m.params[1]), srv.GetClientIdByName(m.params[0])))
        srv.GetChannelByName(m.params[1]).clientsInvited.push_back(srv.GetClientIdByName(m.params[0]));
    rplInviteCmdMsg(srv, client, m);
    return 0;
}

// NICK
// This function handle NICK command; used by a cleint to change 
// his nickname.
int isNickNameInUse(Server & srv, std::string & nickName)
{
    try {
        srv.GetClientIdByName(nickName);
    } catch (...) {
        return 0;
    }
    return 1;
}

int process_nick_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string oldNick;
    std::string rplMsg;

    if (isNickNameInUse(srv, m.params[0]))
    {
        errMsg = ":" + std::string(SRVNAME) + " "
            + std::string(ERR_NICKNAMEINUSE) + " "
            + client->nickname + " " + m.params[0]
            + " :Nickname is already in use\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    oldNick = client->nickname;
    client->nickname = m.params[0];
    rplMsg = ":" + oldNick + " "
        + m.cmd + " "
        + client->nickname + "\r\n";
    srv.BroadCast(rplMsg);
    return 0;
}

// KICK

int notPassedKickCmdLvlOneErr(Server & srv, msg & m, 
    std::vector<Client>::iterator client)
{
    std::string errMsg;

    if (srv.GetChannelIterator(m.params[0]) == srv.GetChannelLastIt())
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOSUCHCHANNEL)
            + " " + client->nickname + " " + m.params[0] + " :No such channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    try {
        srv.GetClientInsideChannel(srv.GetChannelByName(m.params[0]).id, 
            client->id);
    } catch (const std::exception & e)
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_NOTONCHANNEL)
            + " " + client->nickname + " " + m.params[0]
            + " :You're not on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int notPassedKickCmdLvlTwoErr(Server & srv, msg & m, 
    std::vector<Client>::iterator client)
{
    std::string errMsg;

    try {
        srv.GetClientInsideChannel(srv.GetChannelByName(m.params[0]).id,
            srv.GetClientIdByName(m.params[1]));
    } catch (const std::exception & e)
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_USERNOTINCHANNEL)
            + " " + client->nickname + " " + m.params[1] + " " + m.params[0]
            + " :User isn't on that channel\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (!isChanOp(srv.GetChannelByName(m.params[0]), client->id))
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_CHANOPRIVSNEEDED)
            + " " + client->nickname + " " + m.params[0] + " :You're not channel operator\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    if (m.params[1] == client->nickname)
    {
        errMsg = ":" + std::string(SRVNAME) + " " + std::string(ERR_UNKNOWNERROR)
            + " " + client->nickname + " " + m.params[0] + " :Cannot use this command on yourself\r\n";
        srv.SendMsg(client->id, errMsg);
        return -1;
    }
    return 0;
}

int process_kick_cmd(Server & srv, std::vector<Client>::iterator client, msg & m)
{
    std::string errMsg;
    std::string rplMsg;

    if (notPassedKickCmdLvlOneErr(srv, m, client))
        return -1;
    if (notPassedKickCmdLvlTwoErr(srv, m, client))
        return -1;
    if (!isChanOp(srv.GetChannelByName(m.params[0]), srv.GetClientIdByName(m.params[1])))
        srv.RmAClientFromChannel(srv.GetClientIdByName(m.params[1]), srv.GetChannelByName(m.params[0]).id);
    else
        srv.RmAnOpFromChannel(srv.GetClientIdByName(m.params[1]), srv.GetChannelByName(m.params[0]).id);
    rplMsg = srv.getPrefix(*client) + " " + m.cmd + " " + m.params[0]
        + " " + m.params[1] + " :" + (m.params.size() == 2 ? "Misconduct" : m.params[2]) + "\r\n";
    srv.SendMsg(srv.GetClientIdByName(m.params[1]), rplMsg);
    srv.SendMsgToChannel(srv.GetChannelByName(m.params[0]).id, rplMsg);
    return 0;
}

int process_cmds(Server & srv, std::vector<Client>::iterator client, msg & ircMsg)
{
    int error;

    error = 0;
    switch (ircMsg.cmdIdx)
    {
        case 0:
            // handle PASS command;
            // Ignore it because the authentication is done one time 
            // This is already handled in the parsing part; return an error(Already registered)
            // when the client send this command.
            break;
        case 1:
            // handle JOIN command;
            if (process_join_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 2:
            // handle KICK command;
            if (process_kick_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 3:
            break;
        case 4:
            // handle MODE command;
            if (process_mode_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 5:
            // handle PRIVMSG command;
            if (process_privmsg_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 6:
            // handle NOTICE command;
            process_notice_cmd(srv, client, ircMsg);
            break;
        case 7:
            // handle INVITE command;
            if (process_invite_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 8:
            // handle TOPIC command
            if (process_topic_cmd(srv, client, ircMsg))
                error = 1;
            break;
        case 9:
            // handle NICK command;
            if (process_nick_cmd(srv, client, ircMsg))
                error = 1;
            break;
        default:
            break;
    }
    if (error)
        return -1;
    return 0;
}
