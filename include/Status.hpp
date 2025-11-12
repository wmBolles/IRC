/* ============================================================= *
 * Project   : project                                           *
 * File      : Status.hpp                                        *
 * Author    : Yassine Ajagrou                                   *
 * GitHub    : https://github.com/iaceene                        *
 * Created   : 2025-10-15 19:04:56.151277743 +0100               *
 * License   : MIT                                               *
 * ============================================================= */

#ifndef STATUS
#define STATUS

#include "ft_irc.hpp"
#include <string>

#define DEBUG 1

class Status
{
    public :
        Status(const char *str, int LogClr);
        Status(int id, std::string &name, bool state);
        static void PrintTumb();
};

#endif