//
//  Proj2.h
//  Proj2
//
//  Created by Tony on 10/2/16.
//  Copyright © 2016 Anthony Cavallo. All rights reserved.
//

#ifndef Proj1_h
#define Proj1_h

#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

enum State {idle = 0, want_in, in_cs};

#define myKey 18137644

void detachEverything();
void myHandler();
void segFault();


#endif /* Proj1_h */
