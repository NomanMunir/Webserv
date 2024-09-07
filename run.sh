make; valgrind --leak-check=full --show-leak-kinds=all ./webserv config.conf
# --track-origins=yes --track-fds=yes --trace-children=yes