#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/sharedMemory.h"
//#define CHILDS_QTY 4
#define SHM_NAME "sharedMemory\n"
//#define INTIAL_LOAD 1


void * createSharedMemory(char* name,int size, int* fd) {
    *fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (*fd == -1) {
        write(STDERR_FILENO,"failed shm_open",20);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(*fd, size) == -1) {
        write(STDERR_FILENO,"failed ftruncate",20);
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    void *shm_ptrBase= mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if(shm_ptrBase == MAP_FAILED) {
        write(STDERR_FILENO,"failed mmap",20);
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    return shm_ptrBase;
}


int main(int argc, char const *argv[])
{
    int filesQty = argc - 1;
    int shmSize = sizeof(argc) + sizeof(sem_t) + sizeof(message) * filesQty;
    int shmFd;
    void * shm_ptrBase = createSharedMemory(SHM_NAME,shmSize,&shmFd);
    return 0;
}
