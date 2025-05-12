#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

//ca Treasure din header
typedef struct {
    int id;
    char username[50];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }
    char *hunt_id = argv[1];
    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.dat", hunt_id);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("score_calc: open treasures.dat");
        return 1;
    }

    typedef struct { char user[50]; int score; } Entry;
    Entry *list = NULL;
    size_t cnt = 0, cap = 0;
    Treasure t;

    while (read(fd, &t, sizeof(t)) == sizeof(t)) {
        size_t i;
        for (i = 0; i < cnt; i++) {
            if (strcmp(list[i].user, t.username) == 0) {
                list[i].score += t.value;
                break;
            }
        }
        if (i == cnt) {
            if (cnt == cap) {
                cap = cap ? cap * 2 : 8;
                list = realloc(list, cap * sizeof(Entry));
            }
            strcpy(list[cnt].user, t.username);
            list[cnt].score = t.value;
            cnt++;
        }
    }
    close(fd);

    for (size_t i = 0; i < cnt; i++) {
        printf("%s: %d\n", list[i].user, list[i].score);
    }
    free(list);
    return 0;
}