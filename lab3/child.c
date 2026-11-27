#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define MAX_MSG 1024

typedef struct {
    char command[MAX_MSG];
    char response[MAX_MSG];
    int terminate;
} shared_data_t;

long long my_atoll(char *s) {
    long long res = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t' || *s == '\n') {
        s++;
    }
    if (*s == '-') {
        sign = -1; s++;
    }
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0'); s++;
    }
    return res * sign;
}

int is_prime(long long num) {
    if (num < 2) {
        return 0;
    }

    for (long long i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        const char msg[] = "error: invalid arguments\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        const char msg[] = "error: open file failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(argv[1], O_RDWR, 0);
    if (shm_fd < 0) {
        const char msg[] = "error: shm_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    shared_data_t *shm = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        const char msg[] = "error: mmap failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    close(shm_fd);

    sem_t *sem1 = sem_open(argv[2], 0);
    if (sem1 == SEM_FAILED) {
        const char msg[] = "error: sem_open 1 failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem2 = sem_open(argv[3], 0);
    if (sem2 == SEM_FAILED) {
        const char msg[] = "error: sem_open 2 failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(sem1);

        if (shm->terminate) {
            break;
        }

        long long num = my_atoll(shm->command);

        if (num < 0 || is_prime(num)) {
            strcpy(shm->response, "TERMINATE\n");
            shm->terminate = 1;
            sem_post(sem2);
            break;
        }

        if (num == 0 || num == 1) {
            shm->response[0] = '\0';
            sem_post(sem2);
            continue;
        }

        write(fd, shm->command, strlen(shm->command));

        strncpy(shm->response, shm->command, MAX_MSG - 1);
        shm->response[MAX_MSG - 1] = '\0';
        sem_post(sem2);
    }

    close(fd);
    munmap(shm, sizeof(shared_data_t));
    sem_close(sem1);
    sem_close(sem2);

    return 0;
}
