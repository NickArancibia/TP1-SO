#ifndef __SHAREDMEMORY_H
#define __SHAREDMEMORY_H

#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "definitions.h"


typedef struct {
    char md5[MD5LEN+1], filename[MAXFILELEN+1];
    pid_t pid;
} message;
#include "vistaLib.h"

void *createSharedMemory(char* name, int size, int* fd);
void *openSharedMemory(char* name, int size, int* fd);
void closeSharedMemory(void *shm_ptr, int size,int fd);
void destroySharedMemory(const char *name, void *shm_ptr, int fd,int size);

#endif 