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
#include <time.h>

void monitor_list_hunts();
void monitor_handle_usr1(int sig) { monitor_list_hunts(); }
void monitor_handle_usr2(int sig) {
    char buf[512], cmd[512];
    int fd = open("monitor_cmd.txt", O_RDONLY);
    if (fd < 0) return;
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return;
    buf[n] = '\0';
    char *token = strtok(buf, " \n");
    if (!token) return;
    if (strcmp(token, "list_treasures") == 0) {
        char *hunt_id = strtok(NULL, " \n");
        if (hunt_id) {
            snprintf(cmd, sizeof(cmd), "./hunt list %s", hunt_id);
            system(cmd);
        }
    } else if (strcmp(token, "view_treasure") == 0) {
        char *hunt_id = strtok(NULL, " \n");
        char *treasure_id = strtok(NULL, " \n");
        if (hunt_id && treasure_id) {
            pid_t pid = fork();
            if (pid == 0) {
                char *args[] = {"./hunt", "view", hunt_id, treasure_id, NULL};
                execv(args[0], args);
                _exit(1);
            } else if (pid > 0) {
                waitpid(pid, NULL, 0);
            }
        }
    }
    unlink("monitor_cmd.txt");
}
void monitor_handle_term(int sig) { usleep(500000); exit(0); }
void setup_monitor_signals() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = monitor_handle_usr1;
    sigaction(SIGUSR1, &sa, NULL);
    sa.sa_handler = monitor_handle_usr2;
    sigaction(SIGUSR2, &sa, NULL);
    sa.sa_handler = monitor_handle_term;
    sigaction(SIGTERM, &sa, NULL);
}
void monitor_list_hunts() {
    DIR *d = opendir(".");
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_type == DT_DIR &&
            strcmp(e->d_name, ".") && strcmp(e->d_name, "..")) {
            char path[300];// 256 prea mic
            snprintf(path, sizeof(path), "%s/treasures.dat", e->d_name);
            struct stat st;
            if (stat(path, &st) == 0) {
                int count = st.st_size / sizeof(void*); // placeholder size
                printf("Hunt: %s Size: %ld Last modified: %sTreasures: %d\n",
                       e->d_name, st.st_size, ctime(&st.st_mtime), count);
            }
        }
    }
    closedir(d);
}

int main() {
    pid_t monitor_pid = 0;
    char input[256];
    while (1) {
        fflush(stdout);
        printf("treasure_hub> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        char *cmd = strtok(input, " \n");
        if (!cmd) continue;

        /*
    if (!monitor_pid && (
            strcmp(cmd, "list_hunts")       == 0 ||
            strcmp(cmd, "list_treasures")   == 0 ||
            strcmp(cmd, "view_treasure")    == 0 ||
            strcmp(cmd, "calculate_score")   == 0 ||
            strcmp(cmd, "stop_monitor")      == 0
        ))
    {
        printf("Monitor not running.\n");
        continue;
    }

        */

        if (strcmp(cmd, "start_monitor") == 0) {
            if (monitor_pid) {
                printf("Monitor already running.\n");
            } else {
                monitor_pid = fork();
                if (monitor_pid < 0) {
                    perror("fork");
                } else if (monitor_pid == 0) {
                    setup_monitor_signals();
                    while (1) pause();
                } else {
                    printf("Monitor started with PID %d.\n", monitor_pid);
                }
            }
        } else if (strcmp(cmd, "list_hunts") == 0) {
            if (!monitor_pid) {
                printf("Monitor not running.\n");
            } else {
                kill(monitor_pid, SIGUSR1);
            }
        } else if (strcmp(cmd, "list_treasures") == 0) {
            if (!monitor_pid) {
                printf("Monitor not running.\n");
            } else {
                char *hunt_id = strtok(NULL, " \n");
                if (!hunt_id) {
                    printf("Usage: list_treasures <hunt_id>\n");
                } else {
                    int fd = open("monitor_cmd.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
                    dprintf(fd, "list_treasures %s", hunt_id);
                    close(fd);
                    kill(monitor_pid, SIGUSR2);
                }
            }
        } else if (strcmp(cmd, "view_treasure") == 0) {
            if (!monitor_pid) {
                printf("Monitor not running.\n");
            } else {
                char *hunt_id = strtok(NULL, " \n");
                char *treasure_id = strtok(NULL, " \n");
                if (!hunt_id || !treasure_id) {
                    printf("Usage: view_treasure <hunt_id> <treasure_id>\n");
                } else {
                    int fd = open("monitor_cmd.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
                    dprintf(fd, "view_treasure %s %s", hunt_id, treasure_id);
                    close(fd);
                    kill(monitor_pid, SIGUSR2);
                }
            }
        } else if (strcmp(cmd, "stop_monitor") == 0) {
            if (!monitor_pid) {
                printf("Monitor not running.\n");
            } else {
                kill(monitor_pid, SIGTERM);
                waitpid(monitor_pid, NULL, 0);
                monitor_pid = 0;
                printf("Monitor stopped.\n");
            }
        } else if (strcmp(cmd, "calculate_score") == 0) {
	  DIR *d= opendir(".");
	  if(!d){
	    perror("opendir");
	    continue;
	  }
	  struct dirent *e;
	  while ((e = readdir(d)) != NULL) {
	    if (e->d_type == DT_DIR &&
		strcmp(e->d_name, ".") && strcmp(e->d_name, "..")) {
	      char trep[300];//256 nu ajunge
	      snprintf(trep, sizeof(trep), "%s/treasures.dat", e->d_name);
	      if (access(trep, F_OK) != 0) continue;
	      
	      int pfd[2];
	      if (pipe(pfd) < 0) {
		perror("pipe");
		continue;
	      }
	      
	      pid_t cpid = fork();
	      if (cpid < 0) {
		perror("fork");
		close(pfd[0]);
		close(pfd[1]);
		continue;
	      }
	      
	      if (cpid == 0) {
		close(pfd[0]);
		dup2(pfd[1], STDOUT_FILENO);
		close(pfd[1]);
		execlp("./score_calc", "score_calc", e->d_name, NULL);
		perror("execlp score_calc");
		_exit(1);
	      } else {
		close(pfd[1]);
		printf("=== Scores for hunt %s ===\n", e->d_name);
		char line[256];
		FILE *rf = fdopen(pfd[0], "r");
		if (rf) {
		  while (fgets(line, sizeof(line), rf)) {
		    fputs(line, stdout);
		  }
		  fclose(rf);
		} else {
		  perror("fdopen");
		  close(pfd[0]);
		}
		waitpid(cpid, NULL, 0);
	      }
	    }
	  }
	  closedir(d);
	} else if (strcmp(cmd, "exit") == 0) {
            if (monitor_pid) {
                printf("Monitor still running. Stop it before exiting.\n");
            } else {
                break;
            }
        } else if (strcmp(cmd, "help")==0) {
            printf("Command list:\n\"start_monitor\"\n\"list_hunts\"\n\"list_treasures\"   ->   usage: list_treasures <hunt_id>\n\"view_treasure\"   ->   usage: view_treasure <hunt_id> <treasure_id>\n\"calculate_score\"\n\"stop_monitor\"\n\"exit\"\n");
	} else {
            printf("Unknown command. Type \"help\" for a list of commands.\n");
        }
    }
    return 0;
}
