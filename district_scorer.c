//scorer program
//computes workload score for inspectors in one district

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "report.h"

//maximum inspectors we store
#define MAX_INSPECTORS 100


//stores one inspector and total severity score
typedef struct {

    char name[NAME_LEN];
    int score;

} InspectorScore;


int main(int argc, char **argv) {

    if (argc != 2) {

        printf("Usage: district_scorer <district>\n");

        return 1;
    }


    // path to reports.dat
    char path[256];

    snprintf(path,sizeof(path),"%s/reports.dat",argv[1]);


    //open reports file
    int fd = open(path, O_RDONLY);

    if (fd==-1) {

        printf("Could not open %s\n", path);

        return 1;
    }


    //array of inspector scores
    InspectorScore scores[MAX_INSPECTORS];

    int count = 0;


    //report structure
    Report r;


    //read all reports one by one
    while (read(fd,&r,sizeof(Report)) == sizeof(Report)) {

        int found = 0;


        //check if inspector already exists
        for (int i = 0; i < count; i++) {

            if (strcmp(scores[i].name,
                       r.inspector) == 0) {

                //add severity to existing score
                scores[i].score += r.severity;

                found = 1;

                break;
            }
        }


        //new inspector
        if (!found && count < MAX_INSPECTORS) {

            strncpy(scores[count].name,r.inspector,NAME_LEN);

            scores[count].name[NAME_LEN - 1] = '\0';

            scores[count].score = r.severity;

            count++;
        }
    }


    close(fd);


    // print results
    printf("District: %s\n", argv[1]);

    for (int i = 0; i < count; i++) {

        printf("Inspector %s -> workload score %d\n",scores[i].name,scores[i].score);
    }

    printf("\n");


    return 0;
}