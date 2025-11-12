#include "ft_irc.hpp"

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> tokens;
    std::string item;

    std::istringstream iss(s);
    while (std::getline(iss, item, delim))
        tokens.push_back(item);
    return tokens;
}

bool NotValidNameChars(std::string &name)
{
    if (name.find('#') != std::string::npos || name.find('{') != std::string::npos ||
    name.find('}') != std::string::npos || name.find('\t') != std::string::npos ||
    name.find(':') != std::string::npos || name.find('[') != std::string::npos ||
    name.find(']') != std::string::npos || name.find('|') != std::string::npos ||
    name.find('\\') != std::string::npos || name.find('/') != std::string::npos)
        return (false);
    return (true);
}

bool CheckPassword(std::string buffer, std::string ServPass, Server &srv, int id)
{
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    std::vector<std::string> splits = split(buffer, ' ');
    if (splits.empty())
        return (srv.SendMsg(id, ":" + std::string(SRVNAME) + " 461" + " PASS :Not enough parameters\r\n"), false);
    if (splits[0] != "PASS")
        return (srv.SendMsg(id, ":" + std::string(SRVNAME) + " 461" + " PASS :Not enough parameters\r\n"), false);
    if (splits.size() == 1)
        return (srv.SendMsg(id, ":" + std::string(SRVNAME) + " 461" + " PASS :Not enough parameters\r\n"), false);
    std::string pass = splits[1];
    if (pass.empty())
        return (srv.SendMsg(id, ":" + std::string(SRVNAME) + " 461" + " PASS :Not enough parameters\r\n"), false);
    if (pass != ServPass)
        return (srv.SendMsg(id, std::string(SRVNAME) + " 433 :Password incorrect\r\n"), false);
    return (true);
}

bool AddNickname(std::vector<Client>::iterator client, std::string buffer, Server &srv)
{
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    std::vector<std::string> splits = split(buffer, ' ');
    if (splits.empty())
        return (srv.SendMsg((*client).id, ":" + std::string(SRVNAME) + " 461" + " NICK :Not enough parameters\r\n"), false);
    if (splits[0] != "NICK")
        return (srv.SendMsg((*client).id, ":" + std::string(SRVNAME) + " 461" + " NICK :Not enough parameters\r\n"), false);
    if (splits.size() == 1)
        return (srv.SendMsg((*client).id, std::string(SRVNAME) + " 431" + " :No nickname given\r\n"), false);
    if (splits.size() == 2)
    {
        std::string name = splits[1];
        if (name.empty())
            return (srv.SendMsg((*client).id, std::string(SRVNAME) + " 431" + " :No nickname given\r\n"), false);
        if (!NotValidNameChars(name))
            return (srv.SendMsg((*client).id, std::string(SRVNAME) + " 432 " + buffer.substr(buffer.find(' ')) + " :Erroneus nickname\r\n"), false);
        if (!srv.CheckName(name))
            return (srv.SendMsg((*client).id, std::string(SRVNAME) + " 433 " + name + " :Nickname is already in use\r\n"), false);
        (*client).nickname = name;
        return (true);
    }
    if (splits.size() > 2)
        return (srv.SendMsg((*client).id, std::string(SRVNAME) + " 432 " + buffer.substr(buffer.find(' ')) + " :Erroneus nickname\r\n"), false);
    return (false);
}

bool OnlySpaces(std::string &buffer)
{
    size_t count = 0;
    if (buffer.empty())
        return (1);
    for (size_t i = 0; i < buffer.size(); i++)
        if (std::isspace(buffer[i]))
            count++;
    if (count == buffer.size())
        return (1);
    return (0);
}

bool AddUsername(std::vector<Client>::iterator client, std::string buffer, Server &srv)
{
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\r'), buffer.end());
    buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
    std::vector<std::string> splits = split(buffer, ' ');
    if (splits.empty() || splits.size() < 5)
        return (srv.SendMsg((*client).id,  std::string(SRVNAME) + " 461" + " USER :Not enough parameters\r\n"), 0);
    if (splits[0] != "USER" || splits[1].empty() || splits[2] != "0" || splits[3] != "*")
        return (srv.SendMsg((*client).id,  std::string(SRVNAME) + " 461" + " USER :Not enough parameters\r\n"), 0);
    if (splits[4][0] == ':')
        (*client).username = buffer.substr(buffer.find(':') + 1);
    else if (splits.size() == 5)
        (*client).username = splits[4];
    else
        return (srv.SendMsg((*client).id,  std::string(SRVNAME) + " 461" + " USER :Not enough parameters\r\n"), 0);
    if (OnlySpaces((*client).username))
        return (srv.SendMsg((*client).id,  std::string(SRVNAME) + " 461" + " USER :Not enough parameters\r\n"), 0);
    return (1);
}

void Auth(std::string &buffer, std::vector<Client>::iterator client, Server &srv)
{
    if ((*client).auth == -1)
    {
        if (CheckPassword(buffer, srv.GetPass(), srv, (*client).id))
        {
            (*client).auth = 0;
            Status("A client has been entered a correct password", 1);
        }
        else
            Status("A client has been entered a wrong password", 0);
    }
    else if ((*client).auth == 0)
    {
        if (AddNickname(client, buffer, srv))
        {
            (*client).auth = 1;
            Status("A client has been entered a nickname successfully", 1);
        }
        else
            Status("A client has been entered a wrong nickname", 0);
    }
    else if ((*client).auth == 1)
    {
        if (AddUsername(client, buffer, srv))
        {
            (*client).auth = 2;
            Status("A client has been entred a valid username", 1);
        }
        else
            Status("A client has been entred a non valid username", 0);
    }
}

void HandelClient(Server &srv, int id)
{
    std::vector<Client>::iterator client = srv.GetClientById(id);

    Status(std::string(((*client).nickname.empty() ? "Guest" : (*client).nickname) + " sends " + (*client).buffer).c_str(), -1);

    if ((*client).auth != 2 && (*client).auth != 1337)
        Auth((*client).buffer, client, srv);
    if ((*client).auth == 2)
    {
        srv.SendAuthMessage((*client).id);
        (*client).auth = 1337;
    }
    else if ((*client).auth == 1337)
        ProcessLine(srv, client, (*client).buffer);
    (*client).buffer.clear();
}
