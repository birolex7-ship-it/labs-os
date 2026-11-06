#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

long long my_atoll(const char *s) {
    long long res = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res * sign;
}

int is_prime(long long num) {
    if (num < 2) return 0;
    for (long long i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        const char msg[] = "error: file path not provided\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        const char msg[] = "error: failed to open target file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes = 0;
    ssize_t written = 0;

    while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes] = '\0';
        long long num = my_atoll(buffer);

        if (num < 0 || is_prime(num)) {
            const char msg[] = "TERMINATE\n";
            write(STDOUT_FILENO, msg, sizeof(msg) - 1);
            close(fd);
            exit(EXIT_SUCCESS);
        }

        written = write(fd, buffer, bytes);
        if (written != bytes) {
            const char msg[] = "error: failed to write to file\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(fd);
            exit(EXIT_FAILURE);
        }

        written = write(STDOUT_FILENO, buffer, bytes);
        if (written != bytes) {
            const char msg[] = "error: failed to write to stdout\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    return 0;
}
