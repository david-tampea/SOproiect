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
        if (mkdir(hunt_id, 0755) < 0) {
            perror("Error creating hunt directory");
            exit(EXIT_FAILURE);
        }
    }
}

void create_symlink_for_log(const char *hunt_id) {
    char log_path[256];
    char symlink_name[256];
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);
    int fd = open(log_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Error ensuring log file exists");
        exit(EXIT_FAILURE);
    }
    close(fd);
    snprintf(symlink_name, sizeof(symlink_name), "logged_hunt-%s", hunt_id);
    unlink(symlink_name);
    if (symlink(log_path, symlink_name) != 0) {
        perror("Error creating symbolic link for log file");
    }
}

void add_treasure(char *hunt_id) {
    ensure_hunt_directory(hunt_id);
    create_symlink_for_log(hunt_id);
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
    printf("Enter Clue (single word no spaces): ");
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
        printf("Treasure added!\n");
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

void view_treasure(char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    int fp = open(file_path, O_RDONLY);
    if (fp < 0) {
        perror("Error opening treasure file for viewing");
        return;
    }
    Treasure t;
    int found = 0;
    while (read(fp, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == treasure_id) {
            printf("Treasure Details:\n");
            printf("ID: %d\nUser: %s\nLatitude: %.2f\nLongitude: %.2f\nClue: %s\nValue: %d\n",
                   t.id, t.username, t.latitude, t.longitude, t.clue, t.value);
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Treasure with ID %d not found.\n", treasure_id);
    }
    close(fp);
    log_operation(hunt_id, "Viewed treasure");
}

void remove_treasure(char *hunt_id, int treasure_id) {
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/treasures.dat", hunt_id);
    int fp = open(file_path, O_RDONLY);
    if (fp < 0) {
        perror("Error opening treasure file for removal");
        return;
    }
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/temp.dat", hunt_id);
    int fp_temp = open(temp_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fp_temp < 0) {
        perror("Error opening temporary file for removal");
        close(fp);
        return;
    }
    Treasure t;
    int found = 0;
    while (read(fp, &t, sizeof(Treasure)) == sizeof(Treasure)) {
        if (t.id == treasure_id) {
            found = 1;
            continue;
        }
        if (write(fp_temp, &t, sizeof(Treasure)) != sizeof(Treasure)) {
            perror("Error writing to temporary file");
            close(fp);
            close(fp_temp);
            return;
        }
    }
    close(fp);
    close(fp_temp);
    if (!found) {
        printf("Treasure with ID %d not found.\n", treasure_id);
        unlink(temp_path);
    } else {
        if (unlink(file_path) != 0) {
            perror("Error deleting original treasure file");
            return;
        }
        if (rename(temp_path, file_path) != 0) {
            perror("Error renaming temporary file");
            return;
        }
        printf("Treasure with ID %d removed successfully!\n", treasure_id);
        log_operation(hunt_id, "Removed treasure");
    }
}

void remove_hunt(char *hunt_id) {
    char treasure_path[256], log_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/treasures.dat", hunt_id);
    snprintf(log_path, sizeof(log_path), "%s/logged_hunt", hunt_id);
    unlink(treasure_path);
    unlink(log_path);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s/temp.dat", hunt_id);
    unlink(temp_path);
    if (rmdir(hunt_id) != 0) {
        perror("Error removing hunt directory");
        return;
    }
    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "logged_hunt-%s", hunt_id);
    unlink(symlink_path);
    printf("Hunt '%s' removed successfully!\n", hunt_id);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s add <hunt_id>\n", argv[0]);
        printf("  %s list <hunt_id>\n", argv[0]);
        printf("  %s view <hunt_id> <treasure_id>\n", argv[0]);
        printf("  %s remove_treasure <hunt_id> <treasure_id>\n", argv[0]);
        printf("  %s remove_hunt <hunt_id>\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "add") == 0) {
        add_treasure(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_treasures(argv[2]);
    } else if (strcmp(argv[1], "view") == 0) {
        if (argc < 4) {
            printf("Usage: %s view <hunt_id> <treasure_id>\n", argv[0]);
            return 1;
        }
        int treasure_id = atoi(argv[3]);
        view_treasure(argv[2], treasure_id);
    } else if (strcmp(argv[1], "remove_treasure") == 0) {
        if (argc < 4) {
            printf("Usage: %s remove_treasure <hunt_id> <treasure_id>\n", argv[0]);
            return 1;
        }
        int treasure_id = atoi(argv[3]);
        remove_treasure(argv[2], treasure_id);
    } else if (strcmp(argv[1], "remove_hunt") == 0) {
        remove_hunt(argv[2]);
    } else {
        printf("Unknown command: %s\n", argv[1]);
    }
    
    return 0;
}
