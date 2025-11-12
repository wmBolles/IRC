#include "ft_irc.hpp"

bool status;

bool GetRunStatus()
{
    return (status);
}

void SetRunStatus(int val)
{
    status = val;
}

void        CloseServer(int sig)
{
    if (sig == SIGINT || sig == SIGQUIT || sig == SIGSTOP)
        SetRunStatus(0);
}