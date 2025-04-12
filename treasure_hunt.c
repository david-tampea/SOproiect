#include "treasure_hunt_header.h"
#include <fcntl.h>
#include <errno.h>

void log_operation(const char *hunt_id, const char *message) {
    char log_path[256];
    
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);

    
    int fd_log = open(log_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd_log < 0) {
        perror("Error opening/creating log file");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    
    dprintf(fd_log, "[%s] %s\n", ctime(&now), message);
    close(fd_log);
}


void ensure_hunt_directory(char *hunt_id) {
    struct stat st;
    if (stat(hunt_id, &st) != 0) {
        // Presupunem ca e pe linux
        if (mkdir(hunt_id, 0755) < 0) {
            perror("Error creating hunt directory");
            exit(EXIT_FAILURE);
        }
    }
}

void add_treasure(char *hunt_id) {
    ensure_hunt_directory(hunt_id);

    
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    
    int fp = open(file_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fp < 0) {
        perror("Error opening treasure file");
        return;
    }
    
    Treasure t;
    
    printf("Enter Treasure ID (integer): ");
    if (scanf("%d", &t.id) != 1) {
        fprintf(stderr, "Invalid input for treasure id.\n");
        close(fp);
        return;
    }
    
    printf("Enter Username: ");
    scanf("%s", t.username);
    
    printf("Enter Latitude: ");
    if (scanf("%f", &t.latitude) != 1) {
        fprintf(stderr, "Invalid input for latitude.\n");
        close(fp);
        return;
    }
    
    printf("Enter Longitude: ");
    if (scanf("%f", &t.longitude) != 1) {
        fprintf(stderr, "Invalid input for longitude.\n");
        close(fp);
        return;
    }
    
    printf("Enter Clue (single word or no spaces): ");
    scanf("%s", t.clue);
    
    printf("Enter Value (integer): ");
    if (scanf("%d", &t.value) != 1) {
        fprintf(stderr, "Invalid input for value.\n");
        close(fp);
        return;
    }
    
    ssize_t written = write(fp, &t, sizeof(Treasure));
    if (written != sizeof(Treasure)) {
        perror("Failed to write Treasure data");
    } else {
        printf("Treasure added! 🎉\n");
    }
    
    close(fp);
    log_operation(hunt_id, "Added treasure");
}

void list_treasures(char *hunt_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    
    int fp = open(file_path, O_RDONLY);
    if (fp < 0) {
        perror("Error opening treasure file for listing");
        return;
    }
    
    printf("Listing treasures for hunt: %s\n", hunt_id);
    Treasure t;
    
    while (read(fp, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("ID: %d | User: %s | Lat: %.2f | Lon: %.2f | Value: %d\nClue: %s\n---\n",
               t.id, t.username, t.latitude, t.longitude, t.value, t.clue);
    }
    
    close(fp);
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
