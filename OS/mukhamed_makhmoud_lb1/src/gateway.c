#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        perror("Error");
        return 1;
    }

    char buffer[100];
    ssize_t bytesRead;
    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    close(file);
    return 0;
}