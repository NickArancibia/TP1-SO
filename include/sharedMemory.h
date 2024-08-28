#ifndef __SHAREDMEMORY_H
#define __SHAREDMEMORY_H

#include <unistd.h>
#include <semaphore.h>
#include "definitions.h"

typedef struct {
    char md5[MD5LEN+1], filename[MAXFILELEN+1];
    pid_t pid;
} message;

#endif 