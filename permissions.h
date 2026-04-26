#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <sys/types.h>

void mode_to_string(mode_t mode, char *out);
int check_read_permission(const char *path, const char *role);
int check_write_permission(const char *path, const char *role);
int check_exact_permissions(const char *path, mode_t expected);

#endif