NAME = webserv

CXX = c++
CXXFLAGS = -g3 -fsanitize=address #-std=c++98 -Wall -Wextra -Werror 

SRCS = main.cpp \
		Server.cpp \
		ServerManager.cpp \
		parsing/Parser.cpp parsing/Validation.cpp \
		request/Headers.cpp request/Request.cpp request/Body.cpp \
		response/Response.cpp response/HttpResponse.cpp \
		utils/utils.cpp \
		Client.cpp \
		cgi/Cgi.cpp \

		ifeq ($(shell uname), Linux)
			SRCS += events/Epoll.cpp
		else
			SRCS += events/KQueue.cpp
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