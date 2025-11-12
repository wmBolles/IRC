/* ============================================================= *
 * Project   : project                                           *
 * File      : main.cpp                                          *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#include "ft_irc.hpp"

int main(int ac, char **av)
{
    signal(SIGINT, CloseServer);
    signal(SIGQUIT, CloseServer);
    signal(SIGTSTP, CloseServer);
    signal(SIGPIPE, SIG_IGN);
    SetRunStatus(1);
    try
    {
        ft_init(ac, av);
    }
    catch(const std::exception& e)
    {
        std::cerr << RED "➤ Error" RESET << " : " << e.what() << std::endl;
        Status("Programme exited with 1 or by a signal", 0);
        return (1);
    }
    Status("Programme exited with 0", 1);
    return (0);
}