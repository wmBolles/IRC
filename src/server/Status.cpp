/* ============================================================= *
 * Project   : project                                           *
 * File      : Status.cpp                                        *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#include "Status.hpp"

Status::Status(const char *str, int LogClr)
{
    if (DEBUG)
    {
        if (LogClr == 1)
            std::cout << GREEN "➤ Logs" << RESET "  : " << GREEN << str << RESET << std::endl;
        else if (LogClr == -1)
            std::cout << GREEN "➤ Logs" << RESET "  : " << GREEN << str << RESET;
        else
            std::cout << GREEN "➤ Logs" << RESET "  : " << YELLOW << str << RESET << std::endl;
    }
}

Status::Status(int id, std::string &name, bool state)
{
    if (DEBUG)
    {
        if (state)
            std::cout << BLUE "➤ Logs" << RESET "  : " << GREEN << "[" << id
                      << "] " << name << " CONNECTED" << RESET << std::endl;
        std::cout << BLUE "➤ Logs" << RESET "  : " << RED << "[" << id
                  << "] " << name << " DISCONNECTED" << RESET << std::endl;
    }
}

void Status::PrintTumb()
{
    std::cout << " ___ ____   ____   ____                           " << std::endl;
    std::cout << "|_ _|  _ \\ / ___| / ___|  ___ _ ____   _____ _ __ " << std::endl;
    std::cout << " | || |_) | |     \\___ \\ / _ \\ '__\\ \\ / / _ \\ '__|" << std::endl;
    std::cout << " | ||  _ <| |___   ___) |  __/ |   \\ V /  __/ |   " << std::endl;
    std::cout << "|___|_| \\_\\____| |____/ \\___|_|    \\_/ \\___|_|   " << std::endl;
    std::cout << "                                                  " << std::endl;
}