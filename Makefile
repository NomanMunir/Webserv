NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror 

SRCS = main.cpp \
		srcs/server/Server.cpp srcs/server/ServerManager.cpp  \
		srcs/parsing/Parser.cpp srcs/parsing/Validation.cpp \
		srcs/request/Headers.cpp srcs/request/Request.cpp srcs/request/Body.cpp \
		srcs/response/Response.cpp srcs/response/HttpResponse.cpp \
		srcs/utils/utils.cpp srcs/utils/Logs.cpp srcs/utils/MimeTypes.cpp \
		srcs/client/Client.cpp \
		srcs/cgi/Cgi.cpp \

		ifeq ($(shell uname), Linux)
			SRCS += srcs/events/EpollPoller.cpp
		else
			SRCS += srcs/events/KQueuePoller.cpp
		endif

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)
	
fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re