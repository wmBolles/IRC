INCDIR = ./include
ROOTDIR = ./src
SERDIR = $(ROOTDIR)/server
CMDDIR = $(ROOTDIR)/commands
BOTDIR = $(ROOTDIR)/bot

SRCS = $(ROOTDIR)/main.cpp \
		$(SERDIR)/ft_init.cpp \
		$(SERDIR)/help.cpp \
		$(SERDIR)/Status.cpp \
		$(SERDIR)/Server.cpp \
		$(SERDIR)/Auth.cpp \
		$(SERDIR)/Signal.cpp \
		$(CMDDIR)/commands.cpp \
		$(CMDDIR)/check_cmd_params.cpp \
		$(CMDDIR)/process_cmds.cpp
		
BOTSRC = $(BOTDIR)/Bot.cpp \
		$(BOTDIR)/main.cpp

OBJS = $(SRCS:.cpp=.o)
BOTOBJS = $(BOTSRC:.cpp=.o)

NAME = ircserv
BOTNAME = bot
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)

SERVHEADERS = $(INCDIR)/ft_irc.hpp $(INCDIR)/Status.hpp $(INCDIR)/Server.hpp $(INCDIR)/commands.hpp
BOTHEADERS = $(INCDIR)/Bot.hpp

all: $(NAME) 

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(BOTNAME): $(BOTOBJS)
	$(CC) $(CFLAGS) $(BOTOBJS) $(SERDIR)/Status.o -o $(BOTNAME)

%.o: %.cpp $(SERVHEADERS) $(BOTHEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

bonus : $(NAME) $(BOTNAME)

clean:
	rm -f $(OBJS) $(BOTOBJS)

fclean: clean
	rm -f $(NAME) $(BOTNAME)

re: fclean all

