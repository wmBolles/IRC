/* ============================================================= *
 * Project   : project                                           *
 * File      : help.cpp                                          *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#include "ft_irc.hpp"
#include <ctime>

std::string GetTime()
{
    time_t t = time(0);
    tm* now = localtime(&t);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", now);
    return (std::string(buffer));
}

std::string GetDate()
{
    char buffer[80] = {0};
    time_t t = time(0);
    tm* now = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
    return (std::string(buffer));
}

std::string GetTimeAndDate()
{
    return (GetDate() + " " + GetTime());
}

const char *DisplayHelp(int status)
{
    if (status == 1)
        return ("./ircserv <PORT> <PASSWORD>");
    else if (status == 2)
        return ("./ircserv <PORT> <PASSWORD> <---(this args cannot be empty !)");
    if (status == 3)
        return ("./ircserv <PORT <---(must be only degits) > <PASSWORD>");
    if (status == 4)
        return ("./ircserv <PORT <---(Possible Ranges 1024 to 49151) > <PASSWORD>");
    return ("run time error");
}

bool OnlyNums(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!std::isdigit(str[i]))
            return (1);
    return (0);
}