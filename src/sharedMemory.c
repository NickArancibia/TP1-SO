// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/sharedMemory.h"
#include <stdlib.h>

void* mapSharedMemory(const char* name, int size, int* fd, int create) {
    int flags = O_RDWR;
    if (create) {
        flags |= O_CREAT;
    }

    *fd = shm_open(name, flags, 0666);

    if (*fd == -1) {
        ERRORMSG("shm_open");
    }

    if (create) {
        if (ftruncate(*fd, size) == -1) {
            destroySharedMemory(name,NULL,*fd,size);
            ERRORMSG("ftruncate");
        }
    }

    void *shm_ptrBase = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if (shm_ptrBase == MAP_FAILED) {
        if (create) {
            destroySharedMemory(name,shm_ptrBase,*fd,size);
            shm_unlink(name);
        }
        else{
            closeSharedMemory(NULL,*fd,size);
        }
        ERRORMSG("mmap");
    }
    return shm_ptrBase;
}


void* createSharedMemory(char* name, int size, int* fd) {
    return mapSharedMemory(name, size, fd, 1);
}


void* openSharedMemory(char* name, int size, int* fd) {
    return mapSharedMemory(name, size, fd, 0);
}



void closeSharedMemory(void *shm_ptr, int size,int fd) {
    if (shm_ptr != NULL && shm_ptr != MAP_FAILED) {
        munmap(shm_ptr, size);
    }
    if (fd != -1) {
        close(fd);
    }
}


void destroySharedMemory(const char *name, void *shm_ptr, int fd,int size) {
    closeSharedMemory(shm_ptr, fd,size);
    shm_unlink(name);
}