#include "treasure_hunt_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>

void monitor_list_hunts();
void monitor_handle_usr1(int sig) { monitor_list_hunts(); }
void monitor_handle_usr2(int sig) {
    char cmd[256];
    int fd = open("monitor_cmd.txt", O_RDONLY);
    if(fd < 0) return;
    int n = read(fd, cmd, sizeof(cmd)-1);
    if(n > 0) {
       cmd[n] = '\0';
       char *token = strtok(cmd, " \n");
       if(token != NULL) {
         if(strcmp(token, "list_treasures")==0) {
             token = strtok(NULL, " \n");
             if(token) { list_treasures(token); }
         } else if(strcmp(token, "view_treasure")==0) {
             char *hunt_id = strtok(NULL, " \n");
             char *treasure_str = strtok(NULL, " \n");
             if(hunt_id && treasure_str) {
                 int tid = atoi(treasure_str);
                 view_treasure(hunt_id, tid);
             }
         }
       }
    }
    close(fd);
    unlink("monitor_cmd.txt");
}
void monitor_handle_term(int sig) { usleep(500000); exit(0); }
void setup_monitor_signals() {
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = monitor_handle_usr1;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = monitor_handle_usr2;
    sigaction(SIGUSR2, &sa, NULL);
    sa.sa_handler = monitor_handle_term;
    sigaction(SIGTERM, &sa, NULL);
}
void monitor_list_hunts() {
    DIR *d = opendir(".");
    if(!d) return;
    struct dirent *entry;
    while((entry = readdir(d)) != NULL) {
        if(entry->d_type == DT_DIR) {
            if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) continue;
            char treasure_path[256];
            snprintf(treasure_path, sizeof(treasure_path), "%s/treasures.dat", entry->d_name);
            if(access(treasure_path, F_OK)==0) {
                struct stat st;
                if(stat(treasure_path, &st)==0) {
                    int count = st.st_size / sizeof(Treasure);
                    printf("Hunt: %s Size: %ld Last modified: %sTreasures: %d\n",
                           entry->d_name,
                           st.st_size,
                           ctime(&st.st_mtime),
                           count);
                }
            }
        }
    }
    closedir(d);
}

int main() {
    pid_t monitor_pid = 0;
    char input[256];
    while(1) {
        printf("treasure_hub> ");
        if(!fgets(input, sizeof(input), stdin)) break;
        char *cmd = strtok(input, " \n");
        if(!cmd) continue;
        if(strcmp(cmd, "start_monitor")==0) {
            if(monitor_pid != 0) {
                printf("Monitor already running.\n");
            } else {
                monitor_pid = fork();
                if(monitor_pid < 0) {
                    perror("Fork error");
                } else if(monitor_pid==0) {
                    setup_monitor_signals();
                    while(1) pause();
                    exit(0);
                } else {
                    printf("Monitor started with PID %d.\n", monitor_pid);
                }
            }
        } else if(strcmp(cmd, "list_hunts")==0) {
            if(monitor_pid == 0) {
                printf("Monitor not running.\n");
            } else {
                kill(monitor_pid, SIGUSR1);
            }
        } else if(strcmp(cmd, "list_treasures")==0) {
            if(monitor_pid == 0) {
                printf("Monitor not running.\n");
            } else {
                char *hunt_id = strtok(NULL, " \n");
                if(!hunt_id) {
                    printf("Usage: list_treasures <hunt_id>\n");
                    continue;
                }
                int fd = open("monitor_cmd.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
                if(fd < 0) {
                    perror("Error opening monitor_cmd.txt");
                    continue;
                }
                dprintf(fd, "list_treasures %s", hunt_id);
                close(fd);
                kill(monitor_pid, SIGUSR2);
            }
        } else if(strcmp(cmd, "view_treasure")==0) {
            if(monitor_pid == 0) {
                printf("Monitor not running.\n");
            } else {
                char *hunt_id = strtok(NULL, " \n");
                char *treasure_str = strtok(NULL, " \n");
                if(!hunt_id || !treasure_str) {
                    printf("Usage: view_treasure <hunt_id> <treasure_id>\n");
                    continue;
                }
                int fd = open("monitor_cmd.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
                if(fd < 0) {
                    perror("Error opening monitor_cmd.txt");
                    continue;
                }
                dprintf(fd, "view_treasure %s %s", hunt_id, treasure_str);
                close(fd);
                kill(monitor_pid, SIGUSR2);
            }
        } else if(strcmp(cmd, "stop_monitor")==0) {
            if(monitor_pid == 0) {
                printf("Monitor not running.\n");
            } else {
                kill(monitor_pid, SIGTERM);
                waitpid(monitor_pid, NULL, 0);
                monitor_pid = 0;
                printf("Monitor stopped.\n");
            }
        } else if(strcmp(cmd, "exit")==0) {
            if(monitor_pid != 0) {
                printf("Monitor still running. Stop it before exiting.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }
    return 0;
}