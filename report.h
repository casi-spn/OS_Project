#ifndef REPORT_H
#define REPORT_H

#include <time.h>

#define NAME_LEN 32
#define CATEGORY_LEN 32
#define DESC_LEN 128

typedef struct {
    int id;
    char inspector[NAME_LEN];
    double latitude;
    double longitude;
    char category[CATEGORY_LEN];
    int severity;
    time_t timestamp;
    char description[DESC_LEN];
} Report;

void print_report(const Report *r);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);

#endif