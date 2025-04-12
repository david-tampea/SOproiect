#ifndef TREASURE_HUNT_HEADER_H
#define TREASURE_HUNT_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    int id;
    char username[50];
    float latitude;
    float longitude;
    char clue[100];
    int value;
} Treasure;

void add_treasure(char *hunt_id);
void list_treasures(char *hunt_id);
void view_treasure(char *hunt_id, int treasure_id);
void remove_treasure(char *hunt_id, int treasure_id);
void remove_hunt(char *hunt_id);

#endif
