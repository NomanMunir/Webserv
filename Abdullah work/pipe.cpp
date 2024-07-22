#include <iostream>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd = open("specific_fd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    // Duplicate the file descriptor to a specific number (e.g., 10)
    int specific_fd = 10;
    if (dup2(fd, specific_fd) == -1) {
        std::cerr << "Error duplicating file descriptor" << std::endl;
        close(fd);
        return 1;
    }

    // Write to the new file descriptor
    const char *msg = "Hello from the specific file descriptor\n";
    write(specific_fd, msg, strlen(msg));

    // Close file descriptors
    close(fd);
    close(specific_fd);

    return 0;
}
