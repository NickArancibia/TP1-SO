#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "../include/sharedMemory.h"
#include "../include/definitions.h"
#include "../include/vistaLib.h"

int main(int argc, char * argv[]){
    char memName[MAXNAMELEN+1];
    int memfd, dataToRead;
    sem_t * semaphore;
    message * baseData;
    void * basePtr=NULL;

    getMemPath(memName, argc, argv[1]);
    memfd = shm_open(memName, O_RDWR, 0666);
    if(memfd == -1){
        ERRORMSG("shm_open");
    }

	getDataToRead(&dataToRead, memfd);
    int memSize = sizeof(int) + sizeof(message) * dataToRead;

    basePtr = mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
    if(basePtr == MAP_FAILED){
        close(memfd);
        ERRORMSG("basePtr mmap");
    }

    semaphore = sem_open(memName, 0);
    if(semaphore == SEM_FAILED){
        munmap(basePtr, memSize);
        close(memfd);
        ERRORMSG("sem_open");
    }
    baseData = (message *) (basePtr + sizeof(int)); 

    readAndCopyData(dataToRead, memfd, semaphore, baseData);
    sem_close(semaphore);
    munmap(basePtr, memSize);
    close(memfd);
    return 0;
}

