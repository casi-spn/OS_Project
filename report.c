#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

void print_report(const Report *r) {
    char time_str[64];
    struct tm *tm_info;

    tm_info = localtime(&r->timestamp);

    strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", tm_info);

    printf("Report ID: %d\n", r->id);
    printf("Inspector: %s\n", r->inspector);
    printf("GPS: %.6lf %.6lf\n", r->latitude, r->longitude);
    printf("Category: %s\n", r->category);
    printf("Severity: %d\n", r->severity);
    printf("Timestamp: %s\n", time_str);
    printf("Description: %s\n", r->description);
    printf("----------------------------\n");
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    char temp[256];
    char *p1;
    char *p2;

    if (strlen(input) >= sizeof(temp)) {
        return 0;
    }//added length check to avoid buffer overflow

    strcpy(temp, input);

    p1 = strchr(temp, ':');
    if (p1 == NULL) {
        return 0;
    }

    *p1 = '\0';

    p2 = strchr(p1 + 1, ':');
    if (p2 == NULL) {
        return 0;
    }

    *p2 = '\0';

    strcpy(field, temp);//replaced memcpy with strcpy and renamed variables 
    strcpy(op, p1 + 1);
    strcpy(value, p2 + 1);

    return 1;
}

static int compare_int(long a, const char *op, long b) {
    if (strcmp(op, "==") == 0) return a == b;
    if (strcmp(op, "!=") == 0) return a != b;
    if (strcmp(op, "<") == 0) return a < b;
    if (strcmp(op, "<=") == 0) return a <= b;
    if (strcmp(op, ">") == 0) return a > b;
    if (strcmp(op, ">=") == 0) return a >= b;

    return 0;
}

static int compare_string(const char *a, const char *op, const char *b) {
    int cmp = strcmp(a, b);

    if (strcmp(op, "==") == 0) return cmp == 0;
    if (strcmp(op, "!=") == 0) return cmp != 0;

    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        long v = atol(value);
        return compare_int(r->severity, op, v);
    }

    if (strcmp(field, "timestamp") == 0) {
        long v = atol(value);
        return compare_int((long)r->timestamp, op, v);
    }

    if (strcmp(field, "category") == 0) {
        return compare_string(r->category, op, value);
    }

    if (strcmp(field, "inspector") == 0) {
        return compare_string(r->inspector, op, value);
    }

    return 0;
}