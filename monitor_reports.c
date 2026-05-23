// enables POSIX features like sigaction
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

//hidden file where monitor stores its PID
#define PID_FILE ".monitor_pid"

//1 = running
//0 = stop
static volatile sig_atomic_t running = 1;


//handles SIGUSR1
//called when city_manager adds a report
void handle_sigusr1(int sig) {

    //avoid unused parameter warning
    (void)sig;

    //write message to stdout
    //stdout may later be redirected through a pipe
   write(STDOUT_FILENO, "MSG: New report added\n", strlen("MSG: New report added\n"));
}



//handles SIGINT (Ctrl+C) 
//stops monitor and prints shutdown message
void handle_sigint(int sig) {

    (void)sig;

    //print shutdown message
    write(STDOUT_FILENO,"END: Monitor shutting down\n", strlen("END: Monitor shutting down\n"));

    //stop while loop in main
    running = 0;
}


int main() {

    int fd;

    //buffer used for PID and messages
    char buffer[64];

    //current process PID
    pid_t pid = getpid();


    
    //check if another monitor already exists
    

    //try opening existing PID file
    fd = open(PID_FILE, O_RDONLY);

    //if file exists
    if (fd != -1) {

        //read PID from file
        int n = read(fd, buffer, sizeof(buffer) - 1);

        close(fd);

        //if something was read
        if (n > 0) {

            //terminate string
            buffer[n] = '\0';

            //send error message
            printf("ERR: Monitor already running with PID %s", buffer);

            //force immediate output
            fflush(stdout);

            //end program
            return 1;
        }
    }


   
    // create/overwrite .monitor_pid
    fd = open(PID_FILE,O_WRONLY | O_CREAT | O_TRUNC,0644);

    //check error
    if (fd == -1) {

        perror("open");

        return 1;
    }


    //convert PID to string
    snprintf(buffer,sizeof(buffer),"%d\n",pid);

    //write PID into file
    write(fd,buffer,strlen(buffer));

    //close file
    close(fd);


    
    //startup message
    

    printf("MSG: Monitor started with PID %d\n", pid);


    fflush(stdout);//force output


   

    struct sigaction sa_usr1;
    struct sigaction sa_int;



    //function called on SIGUSR1
    sa_usr1.sa_handler = handle_sigusr1;

    //no blocked signals
    sigemptyset(&sa_usr1.sa_mask);

    //no special flags
    sa_usr1.sa_flags = 0;

    //register handler
    sigaction(SIGUSR1,&sa_usr1,NULL);


    //function called on SIGINT
    sa_int.sa_handler = handle_sigint;

    sigemptyset(&sa_int.sa_mask);

    sa_int.sa_flags = 0;

    //register handler
    sigaction(SIGINT,&sa_int,NULL);


    while (running) {

        //wait until signal arrives
        pause();
    }
    
    // delete PID file
    unlink(PID_FILE);

    return 0;
}