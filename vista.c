#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "include/sharedMemory.h"
#include "include/definitions.h"


#define MAXNAMELEN 50
#define ERRORMSG(msg) { perror(msg); exit(1); }

const char * format = "Filename: %s\tPID: %d\tMD5: %s\n";

int main(int argc, char * argv[]){

    char memName[MAXNAMELEN+1];
    int memfd, count=0, * dataLeft, semValue;
    sem_t * semaphore;
    message * baseData;
    void * basePtr=NULL;

    if(argc > 1){
        strcpy(memName, argv[1]);
    }
    else{
        read(STDIN_FILENO, memName, MAXNAMELEN);
    }
    memfd = shm_open(memName, O_RDWR, 0666);
    if(memfd == -1){
        ERRORMSG("shm_open");
    }

    
    basePtr = mmap(NULL, sizeof(int) + sizeof(sem_t) + sizeof(message), PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0);
    if(basePtr == MAP_FAILED){
        ERRORMSG("basePtr mmap");
    }

    dataLeft = (int *) basePtr;
    semaphore = (sem_t *) basePtr + sizeof(int);
    baseData = (message *) (basePtr + sizeof(sem_t) + sizeof(int)); 

    sem_getvalue(semaphore, &semValue);
    while( *dataLeft>0 || (*dataLeft==0 && semValue!=0) ){
        sem_wait(semaphore);
        printf(format, baseData[count].filename, baseData[count].pid, baseData[count].md5);
        count++;
        sem_getvalue(semaphore, &semValue);
    }

    munmap(basePtr, sizeof(int) + sizeof(sem_t) + sizeof(message));
    close(memfd);
    return 0;
}
