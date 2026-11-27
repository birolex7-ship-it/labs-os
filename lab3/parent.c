#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

#define MAX_MSG 1024

typedef struct {
    char command[MAX_MSG];
    char response[MAX_MSG];
    int terminate;
} shared_data_t;

void int_to_str(int num, char *buf) {
    char tmp[16];
    int i = 0, j = 0;
    if (num == 0) {
        buf[0] = '0'; buf[1] = '\0'; return;
    }
    while (num > 0) {
        tmp[i++] = (char)('0' + (num % 10)); num /= 10;
    }
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

int main() {
    char file_name[256];
    ssize_t file_name_len = read(STDIN_FILENO, file_name, sizeof(file_name) - 1);
    if (file_name_len <= 0) {
        const char msg[] = "error: failed to read file name\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    if (file_name[file_name_len - 1] == '\n') {
        file_name[file_name_len - 1] = '\0';
    } else {
        file_name[file_name_len] = '\0';
    }
    if (file_name[0] == '\0') {
        const char msg[] = "error: empty file name\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int pid = (int)getpid();
    char numbuf[16];
    int_to_str(pid, numbuf);

    char shm_name[64] = "/lab3_shm_";
    char sem1_name[64] = "/lab3_sem1_";
    char sem2_name[64] = "/lab3_sem2_";
    strcat(shm_name, numbuf);
    strcat(sem1_name, numbuf);
    strcat(sem2_name, numbuf);

    int shm_fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0644);
    if (shm_fd < 0) {
        const char msg[] = "error: shm_open failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        const char msg[] = "error: ftruncate failed\n";
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

    sem_t *sem1 = sem_open(sem1_name, O_CREAT | O_EXCL, 0644, 0);
    if (sem1 == SEM_FAILED) {
        const char msg[] = "error: sem_open 1 failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem2 = sem_open(sem2_name, O_CREAT | O_EXCL, 0644, 0);
    if (sem2 == SEM_FAILED) {
        const char msg[] = "error: sem_open 2 failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();
    if (child == -1) {
        const char msg[] = "error: fork failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (child == 0) {
        execl("./child", "child", shm_name, sem1_name, sem2_name, file_name, (char *)NULL);
        const char msg[] = "error: execl failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(EXIT_FAILURE);
    }

    char buffer[MAX_MSG];
    ssize_t bytes;

    while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        if (buffer[0] == '\n') {
            break;
        }
        buffer[bytes] = '\0';

        strncpy(shm->command, buffer, MAX_MSG - 1);
        shm->command[MAX_MSG - 1] = '\0';
        shm->terminate = 0;

        sem_post(sem1);
        sem_wait(sem2);

        if (shm->terminate) {
            break;
        }

        write(STDOUT_FILENO, shm->response, strlen(shm->response));
    }

    if (!shm->terminate) {
        shm->terminate = 1;
        sem_post(sem1);
    }

    wait(NULL);

    munmap(shm, sizeof(shared_data_t));
    sem_close(sem1);
    sem_close(sem2);
    sem_unlink(sem2_name);
    sem_unlink(sem1_name);
    shm_unlink(shm_name);

    return 0;
}
