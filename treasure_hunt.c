#include "treasure_hunt_header.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

void log_operation(const char *hunt_id, const char *message) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_id, LOG_FILE);
    FILE *log = fopen(log_path, "a");
    if (log) {
        time_t now = time(NULL);
        fprintf(log, "[%s] %s\n", ctime(&now), message);
        fclose(log);
    }
}

// verifica daca exista, daca nu il creeaza
void ensure_hunt_directory(char *hunt_id) {
    struct stat st;
    if (stat(hunt_id, &st) != 0) {
        if (mkdir(hunt_id, 0755) < 0) {
            perror("Error creating hunt directory");
            exit(EXIT_FAILURE);
        }
    }
}

//nu e gata
void add_treasure(char *hunt_id) {
    ensure_hunt_directory(hunt_id);
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);
    FILE *fp = fopen(file_path, "ab+");
    if (!fp) {
        perror("Error opening treasure file");
        return;
    }
    
    Treasure t;
    printf("Enter Treasure ID: ");
    scanf("%d", &t.id);
    printf("Enter Username: ");
    scanf("%s", t.username);
    printf("Enter Latitude: ");
    scanf("%f", &t.latitude);
    printf("Enter Longitude: ");
    scanf("%f", &t.longitude);
    printf("Enter Clue (single word): ");
    scanf("%s", t.clue);
    printf("Enter Value: ");
    scanf("%d", &t.value);
    
    fwrite(&t, sizeof(Treasure), 1, fp);
    fclose(fp);
    
    log_operation(hunt_id, "Added treasure");
    printf("Treasure added! 🚀\n");
}

// nu e gata
void list_treasures(char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", hunt_id, TREASURE_FILE);
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        perror("Error opening treasure file for listing");
        return;
    }
    
    printf("Listing treasures for hunt: %s\n", hunt_id);
    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, fp)) {
        printf("ID: %d | User: %s | Lat: %.2f | Lon: %.2f | Value: %d\nClue: %s\n---\n",
               t.id, t.username, t.latitude, t.longitude, t.value, t.clue);
    }
    fclose(fp);
    
    log_operation(hunt_id, "Listed treasures");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <command> <hunt_id>\n", argv[0]);
        printf("Commands available: add, list\n");
        return 1;
    }
    
    if (strcmp(argv[1], "add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_treasures(argv[2]);
    } else {
        printf("Unknown command: %s\n", argv[1]);
    }
    
    return 0;
}
