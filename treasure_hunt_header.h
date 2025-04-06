#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdio.h>   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <windows.h>
#include <dirent.h>

//structura comoara
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
