/* ============================================================= *
 * Project   : project                                           *
 * File      : ft_init.cpp                                       *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#include "ft_irc.hpp"

void SetNonBlockingFd(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("(fcntl) failed to get flag");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("(fcntl) failed to set flag");
}

bool OnlySpacePassCheck(std::string str)
{
    for(size_t i = 0; i < str.size(); i++)
        if (!std::isspace(str[i]))
            return (0);
    return (1);
}

void    ft_init(int ac, char **av)
{
    Status::PrintTumb();
    if (ac != 3)
        throw std::runtime_error(DisplayHelp(1));
    else if (std::string(av[1]).empty() || std::string(av[2]).empty() || OnlySpacePassCheck(av[2]))
        throw std::runtime_error(DisplayHelp(2));
    else if (OnlyNums(av[1]))
        throw std::runtime_error(DisplayHelp(3));
    Server srv(av[1], av[2]);
    srv.StartConnection();
}