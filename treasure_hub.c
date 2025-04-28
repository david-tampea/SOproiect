#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    pid_t monitor_pid = 0;
    char input[100];
    while (1) {
        printf("hub> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        if (strncmp(input, "start_monitor", 13) == 0) {
            if (monitor_pid) {
                printf("Monitor already running\n");
            } else {
                monitor_pid = fork();
                if (monitor_pid == 0) {
                    while (1) pause();
                } else if (monitor_pid < 0) {
                    perror("fork");
                } else {
                    printf("Monitor started (PID %d)\n", monitor_pid);
                }
            }
        } else if (strncmp(input, "stop_monitor", 12) == 0) {
            if (!monitor_pid) {
                printf("Monitor not running\n");
            } else {
                kill(monitor_pid, SIGTERM);
                waitpid(monitor_pid, NULL, 0);
                monitor_pid = 0;
                printf("Monitor stopped\n");
            }
        } else if (strncmp(input, "exit", 4) == 0) {
            if (monitor_pid) {
                printf("Stop monitor before exiting\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command\n");
        }
    }
    return 0;
}
