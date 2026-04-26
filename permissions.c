#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "permissions.h"

void mode_to_string(mode_t mode, char *out) {
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';

    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';

    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';

    out[9] = '\0';
}

int check_read_permission(const char *path, const char *role) {
    struct stat st;

    if (stat(path, &st) == -1) {
        perror("stat");
        return 0;
    }

    if (strcmp(role, "manager") == 0 && (st.st_mode & S_IRUSR)) {
        return 1;
    }

    if (strcmp(role, "inspector") == 0 && (st.st_mode & S_IRGRP)) {
        return 1;
    }

    printf("Permission denied: role %s cannot read %s\n", role, path);
    return 0;
}

int check_write_permission(const char *path, const char *role) {
    struct stat st;

    if (stat(path, &st) == -1) {
        perror("stat");
        return 0;
    }

    if (strcmp(role, "manager") == 0 && (st.st_mode & S_IWUSR)) {
        return 1;
    }

    if (strcmp(role, "inspector") == 0 && (st.st_mode & S_IWGRP)) {
        return 1;
    }

    printf("Permission denied: role %s cannot write %s\n", role, path);
    return 0;
}

int check_exact_permissions(const char *path, mode_t expected) {
    struct stat st;

    if (stat(path, &st) == -1) {
        perror("stat");
        return 0;
    }

    if ((st.st_mode & 0777) != expected) {
        printf("Wrong permissions for %s\n", path);
        printf("Expected: %o\n", expected);
        printf("Found: %o\n", st.st_mode & 0777);
        return 0;
    }

    return 1;
}