#include <unistd.h>
#include <fcntl.h>

int my_atoi(const char *s) {
    int res = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res * sign;
}

int is_prime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int read_line(int fd, char *buf, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        int r = read(fd, &c, 1);
        if (r <= 0) break;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

int main() {
    char filename[256];
    if (read_line(0, filename, sizeof(filename)) == 0) {
        return 1;
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return 1; 
    }

    char buf[256];
    while (1) {
        int len = read_line(0, buf, sizeof(buf));
        if (len == 0) break;

        int num = my_atoi(buf);

        if (num < 0 || is_prime(num)) {
            write(1, "E", 1); 
            close(fd);
            return 0;
        }

        write(fd, buf, len);
        write(fd, "\n", 1);
    }

    close(fd);
    return 0;
}
