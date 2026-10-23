#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <poll.h>

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
    int pipe1[2]; 
    int pipe2[2]; 

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        write(2, "Error: pipe failed\n", 19);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        write(2, "Error: fork failed\n", 19);
        return 1;
    }

    if (pid == 0) {
        dup2(pipe1[0], 0);
        dup2(pipe2[1], 1);

        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);

        char *args[] = {"./child", NULL};
        execve("./child", args, NULL);
        
        write(2, "Error: execve failed\n", 21);
        _exit(1);
    } else {
        close(pipe1[0]); 
        close(pipe2[1]); 

        char buf[256];
        write(1, "Enter filename: ", 16);
        int len = read_line(0, buf, sizeof(buf));
        if (len > 0) {
            write(pipe1[1], buf, len);
            write(pipe1[1], "\n", 1);
        }

        struct pollfd fds[2];
        fds[0].fd = 0;          
        fds[0].events = POLLIN;
        fds[1].fd = pipe2[0];   
        fds[1].events = POLLIN;

        while (1) {
            write(1, "Enter number: ", 14);
            
            int poll_res = poll(fds, 2, -1);
            if (poll_res < 0) {
                write(2, "Error: poll failed\n", 19);
                break;
            }

            if (fds[1].revents & (POLLIN | POLLHUP)) {
                break;
            }

            if (fds[0].revents & POLLIN) {
                int l = read_line(0, buf, sizeof(buf));
                if (l == 0) break; 
                
                write(pipe1[1], buf, l);
                write(pipe1[1], "\n", 1);
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);
        
        waitpid(pid, NULL, 0);
        write(1, "Parent process terminated.\n", 27);
    }

    return 0;
}
