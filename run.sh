make; valgrind --track-origins=yes --track-fds=yes --leak-check=full --show-leak-kinds=all ./webserv config.conf
#  --trace-children=yes