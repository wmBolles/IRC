#include "Bot.hpp"

int main(int c, char **v)
{
    if (c != 3)
        return (std::cerr << YELLOW << "Ex. Usage: "<< RESET << "./bot localhost 6667\n", 1);
    try
    {
        Bot bot(v[1], v[2]);
        bot.StartConnection();
        bot.StartListen();
    }
    catch(const std::exception& e)
    {
        std::cerr << RED "➤ Error" RESET << " : " << e.what() << std::endl;
    }    
}