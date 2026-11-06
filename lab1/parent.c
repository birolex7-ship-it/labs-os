#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int starts_with_terminate(const char *str) {
    const char *prefix = "TERMINATE";
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
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

    int parent_to_child[2];
    int child_to_parent[2];
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        const char msg[] = "error: pipe failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        const char msg[] = "error: fork failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        dup2(parent_to_child[0], STDIN_FILENO);
        close(parent_to_child[0]);
        
        dup2(child_to_parent[1], STDOUT_FILENO);
        close(child_to_parent[1]);

        execl("./child", "child", file_name, NULL);
        
        const char msg[] = "error: execl failed\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    close(parent_to_child[0]);
    close(child_to_parent[1]);

    char buffer[1024];
    ssize_t bytes = 0;
    ssize_t written = 0;

    while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        if (buffer[0] == '\n') break;

        written = write(parent_to_child[1], buffer, bytes);
        if (written != bytes) {
            const char msg[] = "error: failed to write to parent_to_child\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }

        bytes = read(child_to_parent[0], buffer, sizeof(buffer) - 1);
        if (bytes < 0) {
            const char msg[] = "error: failed to read from child_to_parent\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        if (bytes == 0) break;
        
        buffer[bytes] = '\0';

        if (starts_with_terminate(buffer)) {
            break;
        }
        
        written = write(STDOUT_FILENO, buffer, bytes);
        if (written != bytes) {
            const char msg[] = "error: failed to write to stdout\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
    }

    close(parent_to_child[1]);
    close(child_to_parent[0]);

    wait(NULL);
    return 0;
}
