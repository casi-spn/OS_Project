#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "report.h"
#include "permissions.h"

#define REPORTS_NAME "reports.dat"
#define CFG_NAME "district.cfg"
#define LOG_NAME "logged_district"

static void build_path(char *out, const char *district, const char *file) {
    sprintf(out, "%s/%s", district, file);
}

static void create_symlink_for_district(const char *district) {
    char linkname[256];
    char target[256];

    sprintf(linkname, "active_reports-%s", district);
    build_path(target, district, REPORTS_NAME);

    unlink(linkname);

    if (symlink(target, linkname) == -1) {
        perror("symlink");
    }
}

static void check_active_link(const char *district) {
    char linkname[256];
    struct stat lst;

    sprintf(linkname, "active_reports-%s", district);

    if (lstat(linkname, &lst) == -1) {
        printf("Warning: active link does not exist: %s\n", linkname);
        return;
    }

    if (!S_ISLNK(lst.st_mode)) {
        printf("Warning: %s exists but is not a symbolic link\n", linkname);
        return;
    }

    if (stat(linkname, &lst) == -1) {
        printf("Warning: dangling symbolic link: %s\n", linkname);
    }
}

static void create_district_if_needed(const char *district) {
    char path[256];
    int fd;
    struct stat st;

    mkdir(district, 0750);
    chmod(district, 0750);

    build_path(path, district, REPORTS_NAME);
    fd = open(path, O_CREAT | O_RDWR, 0664);
    if (fd == -1) {
        perror("open reports.dat");
        exit(1);
    }
    close(fd);
    chmod(path, 0664);

    build_path(path, district, CFG_NAME);
    fd = open(path, O_CREAT | O_RDWR, 0640);
    if (fd == -1) {
        perror("open district.cfg");
        exit(1);
    }

    if (stat(path, &st) == 0 && st.st_size == 0) {
        write(fd, "threshold=2\n", 12);
    }

    close(fd);
    chmod(path, 0640);

    build_path(path, district, LOG_NAME);
    fd = open(path, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open logged_district");
        exit(1);
    }
    close(fd);
    chmod(path, 0644);

    create_symlink_for_district(district);
}

static void log_action(const char *district, const char *role, const char *user, const char *action) {
    char path[256];
    char buffer[512];
    time_t now;
    int fd;
    int len;

    build_path(path, district, LOG_NAME);

    if (strcmp(role, "manager") != 0) {
        printf("Log warning: inspector role cannot write to logged_district.\n");
        return;
    }

    if (!check_write_permission(path, role)) {
        return;
    }

    fd = open(path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open log");
        return;
    }

    now = time(NULL);

    len = sprintf(buffer, "%ld role=%s user=%s action=%s\n",
                  (long)now, role, user, action);

    write(fd, buffer, len);
    close(fd);
}

static int next_report_id(const char *district) {
    char path[256];
    int fd;
    Report r;
    int max_id = 0;

    build_path(path, district, REPORTS_NAME);

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id > max_id) {
            max_id = r.id;
        }
    }

    close(fd);

    return max_id + 1;
}

static void add_report(const char *district, const char *role, const char *user) {
    char path[256];
    int fd;
    Report r;

    create_district_if_needed(district);
    check_active_link(district);

    build_path(path, district, REPORTS_NAME);

    if (!check_write_permission(path, role)) {
        return;
    }

    r.id = next_report_id(district);

    strncpy(r.inspector, user, NAME_LEN);
    r.inspector[NAME_LEN - 1] = '\0';

    printf("Latitude: ");
    scanf("%lf", &r.latitude);

    printf("Longitude: ");
    scanf("%lf", &r.longitude);

    printf("Category: ");
    scanf("%31s", r.category);

    printf("Severity 1-3: ");
    scanf("%d", &r.severity);

    getchar();

    printf("Description: ");
    fgets(r.description, DESC_LEN, stdin);
    r.description[strcspn(r.description, "\n")] = '\0';

    r.timestamp = time(NULL);

    fd = open(path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open reports.dat");
        return;
    }

    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) {
        perror("write report");
    }

    close(fd);
    chmod(path, 0664);

    printf("Report added with ID %d\n", r.id);

    log_action(district, role, user, "add_report");
}

static void list_reports(const char *district, const char *role) {
    char path[256];
    int fd;
    Report r;
    struct stat st;
    char perms[10];

    check_active_link(district);

    build_path(path, district, REPORTS_NAME);

    if (!check_read_permission(path, role)) {
        return;
    }

    if (stat(path, &st) == -1) {
        perror("stat reports.dat");
        return;
    }

    mode_to_string(st.st_mode, perms);

    printf("reports.dat information:\n");
    printf("Permissions: %s\n", perms);
    printf("Size: %ld bytes\n", (long)st.st_size);
    printf("Last modification time: %ld\n", (long)st.st_mtime);
    printf("\nReports:\n");

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open reports.dat");
        return;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("ID=%d inspector=%s category=%s severity=%d\n",
               r.id, r.inspector, r.category, r.severity);
    }

    close(fd);
}

static void view_report(const char *district, const char *role, int id) {
    char path[256];
    int fd;
    Report r;
    int found = 0;

    check_active_link(district);

    build_path(path, district, REPORTS_NAME);

    if (!check_read_permission(path, role)) {
        return;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open reports.dat");
        return;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == id) {
            print_report(&r);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Report not found.\n");
    }

    close(fd);
}

static void remove_report(const char *district, const char *role, const char *user, int id) {
    char path[256];
    int fd;
    Report r;
    off_t pos = 0;
    off_t next_pos;
    int found = 0;

    if (strcmp(role, "manager") != 0) {
        printf("Only manager can remove reports.\n");
        return;
    }

    check_active_link(district);

    build_path(path, district, REPORTS_NAME);

    if (!check_write_permission(path, role)) {
        return;
    }

    fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("open reports.dat");
        return;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == id) {
            found = 1;
            break;
        }
        pos += sizeof(Report);
    }

    if (!found) {
        printf("Report not found.\n");
        close(fd);
        return;
    }

    next_pos = pos + sizeof(Report);

    while (1) {
        ssize_t bytes;

        lseek(fd, next_pos, SEEK_SET);
        bytes = read(fd, &r, sizeof(Report));

        if (bytes != sizeof(Report)) {
            break;
        }

        lseek(fd, pos, SEEK_SET);
        write(fd, &r, sizeof(Report));

        pos += sizeof(Report);
        next_pos += sizeof(Report);
    }

    ftruncate(fd, pos);
    close(fd);

    printf("Report removed.\n");

    log_action(district, role, user, "remove_report");
}

static void update_threshold(const char *district, const char *role, const char *user, int value) {
    char path[256];
    int fd;
    char buffer[64];
    int len;

    if (strcmp(role, "manager") != 0) {
        printf("Only manager can update threshold.\n");
        return;
    }

    build_path(path, district, CFG_NAME);

    if (!check_exact_permissions(path, 0640)) {
        printf("Refusing to update threshold because permissions are incorrect.\n");
        return;
    }

    if (!check_write_permission(path, role)) {
        return;
    }

    fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("open district.cfg");
        return;
    }

    len = sprintf(buffer, "threshold=%d\n", value);
    write(fd, buffer, len);

    close(fd);

    printf("Threshold updated.\n");

    log_action(district, role, user, "update_threshold");
}

static void filter_reports(const char *district, const char *role, int cond_count, char **conditions) {
    char path[256];
    int fd;
    Report r;

    check_active_link(district);

    build_path(path, district, REPORTS_NAME);

    if (!check_read_permission(path, role)) {
        return;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open reports.dat");
        return;
    }

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int ok = 1;

        for (int i = 0; i < cond_count; i++) {
            char field[64];
            char op[8];
            char value[128];

            if (!parse_condition(conditions[i], field, op, value)) {
                printf("Invalid condition: %s\n", conditions[i]);
                ok = 0;
                break;
            }

            if (!match_condition(&r, field, op, value)) {
                ok = 0;
                break;
            }
        }

        if (ok) {
            print_report(&r);
        }
    }

    close(fd);
}

static void usage(void) {
    printf("Usage:\n");
    printf("./city_manager --role ROLE --user USER --add DISTRICT\n");
    printf("./city_manager --role ROLE --user USER --list DISTRICT\n");
    printf("./city_manager --role ROLE --user USER --view DISTRICT ID\n");
    printf("./city_manager --role ROLE --user USER --remove_report DISTRICT ID\n");
    printf("./city_manager --role ROLE --user USER --update_threshold DISTRICT VALUE\n");
    printf("./city_manager --role ROLE --user USER --filter DISTRICT condition...\n");
}

int main(int argc, char **argv) {
    char *role = NULL;
    char *user = NULL;

    if (argc < 6) {
        usage();
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        }
    }

    if (role == NULL || user == NULL) {
        printf("Error: --role and --user are required.\n");
        return 1;
    }

    if (strcmp(role, "manager") != 0 && strcmp(role, "inspector") != 0) {
        printf("Error: role must be manager or inspector.\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--add") == 0 && i + 1 < argc) {
            add_report(argv[i + 1], role, user);
            return 0;
        }

        if (strcmp(argv[i], "--list") == 0 && i + 1 < argc) {
            list_reports(argv[i + 1], role);
            return 0;
        }

        if (strcmp(argv[i], "--view") == 0 && i + 2 < argc) {
            view_report(argv[i + 1], role, atoi(argv[i + 2]));
            return 0;
        }

        if (strcmp(argv[i], "--remove_report") == 0 && i + 2 < argc) {
            remove_report(argv[i + 1], role, user, atoi(argv[i + 2]));
            return 0;
        }

        if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc) {
            update_threshold(argv[i + 1], role, user, atoi(argv[i + 2]));
            return 0;
        }

        if (strcmp(argv[i], "--filter") == 0 && i + 2 < argc) {
            filter_reports(argv[i + 1], role, argc - i - 2, &argv[i + 2]);
            return 0;
        }
    }

    usage();
    return 1;
}   