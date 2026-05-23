// interactive hub program
// communicates with monitor_reports
// launches scorer processes

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 512
#define MAX_DISTRICTS 20



//starts monitor in background
//creates pipe for monitor output

static void start_monitor(void) {

    //pipe:
    //pipefd[0] = read end
    //pipefd[1] = write end
    int pipefd[2];

    if (pipe(pipefd) == -1) {

        perror("pipe");

        return;
    }


    //create background process
    pid_t hub_mon = fork();

    if (hub_mon == -1) {

        perror("fork");

        return;
    }


   //CHILD = hub_mon 
    if (hub_mon == 0) {

        //second fork for monitor
        pid_t mon = fork();

        if (mon == -1) {

            perror("fork");

            exit(1);
        }


        
        // MONITOR PROCESS
        if (mon == 0) {

            //monitor only writes
            close(pipefd[0]);


            //redirect stdout to pipe
            dup2(pipefd[1],
                 STDOUT_FILENO);

            close(pipefd[1]);


            //replace process with monitor_reports
            execl("./monitor_reports",
                  "monitor_reports",
                  NULL);


            //exec failed
            perror("execl");

            exit(1);
        }


        //hub_mon reads messages
       

        close(pipefd[1]);

        char buffer[256];

        ssize_t n;


        //continuously read monitor output
        while ((n = read(pipefd[0],buffer,sizeof(buffer) - 1)) > 0) {

            buffer[n] = '\0';

            printf("\n[MONITOR] %s",
                   buffer);

            fflush(stdout);


            //monitor ended
            if (strncmp(buffer, "END:",4) == 0) {

                printf("[HUB] Monitor ended.\n");
            }


            //monitor failed
            if (strncmp(buffer,"ERR:",4) == 0) {

                printf("[HUB] Monitor could not start.\n");
            }
        }


        close(pipefd[0]);

        exit(0);
    }


    //PARENT = city_hub
 
    close(pipefd[0]);
    close(pipefd[1]);

    printf("hub: monitor started in background\n");
}




//calculates workload scores
//launches one scorer per district

static void calculate_scores(char **districts,int count) {

    int pipes[MAX_DISTRICTS][2];

    pid_t pids[MAX_DISTRICTS];


    //create scorer for each district
    for (int i = 0; i < count; i++) {

        if (pipe(pipes[i]) == -1) {

            perror("pipe");

            continue;
        }


        pids[i] = fork();

        if (pids[i] == -1) {

            perror("fork");

            continue;
        }


       
        //CHILD = scorer
        
        if (pids[i] == 0) {

            close(pipes[i][0]);


            //redirect stdout to pipe
            dup2(pipes[i][1],
                 STDOUT_FILENO);

            close(pipes[i][1]);


            //execute scorer
            execl("./district_scorer",
                  "district_scorer",
                  districts[i],
                  NULL);

            perror("execl");

            exit(1);
        }


        //parent only reads
        close(pipes[i][1]);
    }




    printf("\nCombined workload report:\n");
    printf("-------------------------\n");


    for (int i = 0; i < count; i++) {

        char buffer[1024];

        ssize_t n;


        while ((n = read(pipes[i][0],buffer,sizeof(buffer) - 1)) > 0) {

            buffer[n] = '\0';

            printf("%s", buffer);
        }


        close(pipes[i][0]);

        waitpid(pids[i],NULL,0);
    }
}




int main(void) {

    char line[MAX_LINE];


    printf("City Hub started.\n");

    printf("Commands:\n");

    printf("  start_monitor\n");

    printf("  calculate_scores <districts>\n");

    printf("  exit\n");


    while (1) {

        printf("hub> ");

        fflush(stdout);


        //read user input
        if (fgets(line,
                  sizeof(line),
                  stdin) == NULL) {

            break;
        }


        //remove newline
        line[strcspn(line, "\n")] = '\0';


        //exit command
        if (strcmp(line, "exit") == 0) {

            break;
        }


        //start monitor
        if (strcmp(line,"start_monitor") == 0) {

            start_monitor();

            continue;
        }


        //calculate scores
        if (strncmp(line,"calculate_scores",16) == 0) {

            char *districts[MAX_DISTRICTS];

            int count = 0;


            //split command
            char *token = strtok(line, " ");

            token = strtok(NULL, " ");


            while (token != NULL && count < MAX_DISTRICTS) {

                districts[count++] = token;

                token = strtok(NULL, " ");
            }


            if (count == 0) {

                printf("Usage: calculate_scores <districts>\n");

            } else {

                calculate_scores(districts,count);
            }

            continue;
        }


        // unknown command
        printf("Unknown command\n");
    }


    printf("City Hub ended.\n");

    return 0;
}