NAME = webserv

CXX = c++
CXXFLAGS = #-std=c++98 -Wall -Wextra -Werror 

SRCS = main.cpp Server.cpp \
		parsing/Parser.cpp parsing/Validation.cpp \
		headers/Headers.cpp headers/Request.cpp headers/Body.cpp \
		utils/utils.cpp \

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