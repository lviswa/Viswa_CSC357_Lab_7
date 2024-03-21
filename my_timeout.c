#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static void handle_alarm(int sig) {
    printf("Killing child ...\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <timeout in seconds> <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int timeout = atoi(argv[1]);
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    } else if (pid == 0) {
        execvp(argv[2], &argv[2]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        struct sigaction sa;
        sa.sa_handler = &handle_alarm;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGALRM, &sa, NULL) == -1) {
            perror("sigaction");
            return EXIT_FAILURE;
        }

        alarm(timeout);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return EXIT_SUCCESS;
        } else {
            if (kill(pid, 0) == -1) {
                return WEXITSTATUS(status);
            } else {
                kill(pid, SIGKILL);
                return EXIT_FAILURE;
            }
        }
    }
}
