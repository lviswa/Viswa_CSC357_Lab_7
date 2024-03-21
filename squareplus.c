#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {
    int p1[2], p2[2], p3[2];
    int v;

    if (pipe(p1) < 0 || pipe(p2) < 0 || pipe(p3) < 0) {
        perror("Pipe creation failed");
        exit(1);
    }

    pid_t c1 = fork();
    if (c1 < 0) {
        perror("Fork 1 failed");
        exit(1);
    } else if (c1 == 0) {
        close(p1[1]); close(p2[0]); close(p3[0]); close(p3[1]);

        while (read(p1[0], &v, sizeof(v)) > 0) {
            v = v * v;
            write(p2[1], &v, sizeof(v));
        }

        close(p1[0]); close(p2[1]);
        exit(0);
    }

    pid_t c2 = fork();
    if (c2 < 0) {
        perror("Fork 2 failed");
        exit(1);
    } else if (c2 == 0) {
        close(p1[0]); close(p1[1]); close(p2[1]); close(p3[0]);

        while (read(p2[0], &v, sizeof(v)) > 0) {
            v = v + 1;
            write(p3[1], &v, sizeof(v));
        }

        close(p2[0]); close(p3[1]);
        exit(0);
    }

    close(p1[0]); close(p2[0]); close(p2[1]); close(p3[1]);

    while (scanf("%d", &v) != EOF) {
        write(p1[1], &v, sizeof(v));
        read(p3[0], &v, sizeof(v));
        printf("Result: %d\n", v);
    }

    close(p1[1]); close(p3[0]);

    int status;
    waitpid(c1, &status, 0);
    waitpid(c2, &status, 0);

    return 0;
}