//
//  main.c
//  Slave
//
//  Created by Tony on 9/24/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.

#include "Proj2.h"

char *filename;
static int shmid;
static int *num;
static int *turn;
static int *flag;

int main(int argc, const char *argv[]) {
    signal(SIGINT, myHandler);
    signal(SIGSEGV, segFault);
    /****** NOTE: ******
     argv[0] is the process id (1 through max slaves)
     argv[1] is the file name
     argv[2] is the number of increments each slave needs to perform
     argv[3] is the shared memory size in bytes
     *******************/
    filename = malloc(sizeof(char *));
    filename = (char*)argv[1];
    
    int i = 0, max_writes = atoi(argv[2]), shmSize = atoi(argv[3]), n = ((shmSize / 4) - 2);
    
    //Take some info from main to access the correct shared memory.
    shmid = shmget(myKey, shmSize, 0777);
    if (shmid < 0) {
        fprintf(stderr, "Error: shmid invalid\n");
        return -1;
    }
    
    if((num = (int *)shmat(shmid, NULL, 0)) == (int *)(-1)) {
        fprintf(stderr, "Error: process %i not attaching to turn in shared memory.\n", atoi(argv[0]));
        return -1;
    }
    turn = num + 1;     //num starts off the shared memory at 0, so we need to do some memory arithmetic for turn and flag.
    flag = num + 2;     //flag will be handled by array notation. flag[ID] where ID = 0 is flag[0]. This is basic pointer/array notation.
                        //So just use ID when referencing the array variable that's needed.
    
    const int ID = atoi(argv[0])-1;
    *turn = ID;
    
    //Implement Peterson's algorithm and hope for the best.
    srand((unsigned int)time(NULL));
    int j;
    time_t now;
    for (i = 0; i < max_writes; i++) {
        //Code to enter critical section
        do {
            flag[ID] = want_in;
            j = *turn;
            while (j != ID)
                j = (flag[j] != idle) ? *turn : (j + 1) % n;
            
            flag[ID] = in_cs;
            for (j = 0; j < n; j++)
                if ((j != ID) && (flag[j] == in_cs))
                    break;
            
        } while ((j < n) || (*turn != ID && flag[*turn] != idle));
        *turn = ID;
        /******Critical Section!******/
        sleep(rand() % 3);      //Sleep for random time before incrementing shared number.
        printf("Entering critical section\n");
        
        time(&now);
        struct tm *myTime = localtime(&now);
        char buff[6];
        strftime(buff, 6, "%H:%M", myTime);
        (*num)++;
        printf("File modified by process number %s at time %s with sharedNum = %i\n", argv[0], buff, *num);
        FILE *file;
        file=fopen(argv[1], "a");
        if (file == NULL) {
            errno = ENOENT;
            return -1;
        }
        fprintf(file, "File modified by process number %s at time %s with sharedNum = %i\n", argv[0], buff, *num);
        fclose(file);
        sleep(rand() % 3);
        
        /******End Critical Section!******/
        //Code to exit critical section
        j = (*turn + 1) % n;
        while (flag[j] == idle)
            j = (j + 1) % n;
        
        *turn = j;
        flag[ID] = idle;
    }
    
    
    detachEverything();
    return 0;
}

void myHandler() {
    detachEverything();
    printf("\nAlas, I am dying! Master, I have been killed!\n");
    pid_t pid = getpid();
    fprintf(stderr, "Slave process %i dying because of an interrupt.\n", pid);
    exit(0);
}

void segFault() {
    detachEverything();
    printf("\nSegmentation fault: YOU DONE MESSED UP A-A-RON!\n");
    pid_t pid = getpid();
    fprintf(stderr, "Slave process %i dying because of a segmentation fault.\n", pid);
    exit(0);
}

void detachEverything() {
    shmdt(num);
    shmdt(turn);
    shmdt(flag);
}
