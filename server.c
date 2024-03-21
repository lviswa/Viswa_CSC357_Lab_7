#define _GNU_SOURCE

#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define PORT 33624 

void sigchld_handler(int s) {
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void handle_request(int nfd)
{
    FILE *network = fdopen(nfd, "r");
    char *line = NULL;
    size_t size;
    ssize_t num;

    if (network == NULL)
    {
        perror("fdopen");
        close(nfd);
        return;
    }

    while ((num = getline(&line, &size, network)) >= 0)
    {
        write(nfd, line, num);
    }

    free(line);
    fclose(network);
}

void run_service(int fd)
{
    signal(SIGCHLD, sigchld_handler);
    while (1)
    {
        int nfd = accept_connection(fd);
        if (nfd != -1)
        {
            pid_t pid = fork();
            if (pid == 0) { 
                close(fd); 
                handle_request(nfd);
                close(nfd);
                exit(0);
            } 
            else if (pid > 0) { 
                close(nfd); 
                printf("Spawned child process %d for new connection\n", pid);
            } 
            else {
                perror("fork");
                close(nfd);
            }
        }
    }
}

int main(void)
{
    int fd = create_service(PORT);
    if (fd == -1)
    {
        perror(0);
        exit(1);
    }

    printf("Listening on port: %d\n", PORT);
    run_service(fd);
    close(fd);

    return 0;
}