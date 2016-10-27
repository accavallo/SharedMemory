//  main.c
//  Proj2
//
//  Created by Tony on 9/19/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.

#include "Proj2.h"

static int sharedMem;
static int *num;    //0 in shared memory
static int *turn;   //num + 1 in shared memory
static int *flag;   //starts at num + 2 in shared memory, but will correspond to each slave process id. Array notation is used to access the
//correct space in memory for each process.

int main(int argc, const char * argv[]) {
    signal(SIGSEGV, segFault);
    int option, spawns = 5, increments = 3, term = 20;
    char *fileName = "test.out";
    
    while ((option = getopt(argc, (char **)argv, "hs:l:i:t:")) != -1) {
        switch (option) {
            case 'h':
                printUsage();
                break;
            case 's':
                if (isdigit(*optarg)) {
                    printf("Setting number of slaves to be spawned to %i.\n", atoi(optarg));
                    spawns = atoi(optarg);
                } else {
                    printf("Invalid argument for parameter '-s'.\nExpected Integer, found '%s' instead.\n", optarg);
                }
                break;
            case 'l':
                printf("Setting file name to %s\n", optarg);
                fileName = optarg;
                break;
            case 'i':
                if (isdigit(*optarg)) {
                    printf("Setting the number of times each slave will write to %i.\n", atoi(optarg));
                    increments = atoi(optarg);
                } else {
                    printf("Invalid argument for parameter '-i'.\nExpected Integer, found '%s' instead.\n", optarg);
                }
                break;
            case 't':
                if (isdigit(*optarg)) {
                    printf("Setting amount of time master will wait before terminating itself to %i seconds.\n", atoi(optarg));
                    term = atoi(optarg);
                } else {
                    printf("Invalid argument for parameter '-t'.\nExpected Integer, found '%s' instead.\n", optarg);
                }
                break;
            case '?':
                if (optopt == 'l' || optopt == 's' || optopt == 'i' || optopt == 't') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt)) {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                }
                else {
                    fprintf(stderr, "Unknown option character '%s'.\n", argv[optind-1]);
                }
                break;
            default:
                printf("Not a valid option, printing usage.\n");
                printUsage();
        }
    }
    
    printf("Slaves to be spawned: %i\nNumber of writes: %i\nMaster termination time: %i seconds\n", spawns, increments, term);
    
    //The size of the shared memory will need to be the number of spawns + turn + num to be incremented, so sizeof(int) * (spawns + 2).
    //This information sent to the slave as argv[3].
    sharedMem = shmget(myKey, sizeof(int) * (spawns + 2), IPC_CREAT | 0777);
    if(sharedMem == -1) {
        fprintf(stderr, "Error creating shared memory, program exiting.\n");
        return -1;
    }
    
    if((num = (int *)shmat(sharedMem, NULL, 0)) == (int *)(-1)) {
        fprintf(stderr, "Error: Maaaaster process not attached to turn in shared memory.\n");
        return -1;
    }
    
    turn = num + 1;
    flag = num + 2;
    *num = 0;
    *turn = 0;
    int i;
    //Everything *should* be set to 0, but I'm making sure it is.
    for(i = 0; i < spawns; i++) {
        flag[i] = idle;
    }
    
    //This section is just to test out the funtionality of making a while loop continue based on a set amount of time.
    time_t now = time(NULL);
    time_t endTime = time(NULL);
    pid_t pid, wpid;
    int status, processCount = 0, count = 0;
    char *procID, *max_writes, *shmSize;
    procID = (char*)malloc(sizeof(char) * 10);
    max_writes = (char*)malloc(sizeof(char)*2);
    shmSize = (char*)malloc(sizeof(char*));
    
    signal(SIGINT, myHandler);
    int index = 0;
    sprintf(max_writes, "%i", increments);
    sprintf(shmSize, "%lu", sizeof(int) * (spawns + 2));
    
    while((endTime - now) < term) {
        if ((endTime - now) >= term) {
            myHandler();
        }
        
        if (count < spawns && processCount < 20) {
            pid = fork();
            processCount++;
            count++;
            index++;
        }
        
        if (pid == 0) {
            sprintf(procID, "%i", (count));
            execl("./slave", procID, fileName, max_writes, shmSize, (char *)NULL);
            _exit(EXIT_FAILURE);
        }
        
        if ( WIFEXITED(status) == 0) 
            processCount--;
        
        
        endTime = time(NULL);
    }
    
    //In case any process is still trying to go.
    while ((wpid = wait(&status)) > 0);
    
    printf("Finished waiting.\n");
    detachEverything();
    return 0;
}

//Method to print the help message in case I need to use it more than the two times I have it already.
void printUsage () {
    printf("-h displays this help message\n");
    printf("-l filename sets the filename to be used for the log file\n");
    printf("-s x sets the Integer argument 'x' to the variable 's' for the number of slave\n   processes spawned. By default x is set to 5.\n");
    printf("-i y sets the Integer argument 'y' to the variable 'i' for the number of times\n   each slave will increment and write to a file before terminating. By default y is set to 3.\n");
    printf("-t z sets the Integer argument 'z' to the variable 't' for the amount of time,\n   in seconds, when the master will terminate itself. By default z is set to 20.\n");
}

void myHandler() {
    printf("\nKilling program!\n");
    pid_t groupID = getpgrp();
    
    detachEverything();
    printf("Maaaaster sending signal to kill children. Maaaaster process id: %i\n", getpid());
    killpg(groupID, SIGINT);
    exit(0);
}

void segFault() {
    detachEverything();
    printf("Segmentation fault: YOU DONE MESSED UP A-A-RON!\n");
    pid_t gpid = getpgrp();
    fprintf(stderr, "Master process is dying because of segmentation fault.\n");
    killpg(gpid, SIGSEGV);
    exit(0);
}

void detachEverything() {
    shmdt(num);
    shmdt(turn);
    shmdt(flag);
    shmctl(sharedMem, IPC_RMID, NULL);
}
