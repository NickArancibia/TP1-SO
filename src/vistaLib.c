#include "../include/vistaLib.h"
#include <stdio.h>

const char * format = "Filename: %s - PID: %d - MD5: %s\n";

void getMemPath(char * memName, int argc, const char * argv1){
    if(memName == NULL)
        return;
    if(argc > 1){
        strncpy(memName, argv1, MAXNAMELEN);
    }
    else{
        read(STDIN_FILENO, memName, MAXNAMELEN);
    }
}

void getDataToRead(int * dataLeft, int memoryFd){
    if(dataLeft == NULL)
        return;
    int * basePointer = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, memoryFd, 0);
    *dataLeft = *basePointer;
    munmap(basePointer, sizeof(int));
}

void readAndCopyData(int dataLeft, int memoryFd, sem_t * semaphore, message * baseData){
    int count = 0;
    while( count != dataLeft ){
        sem_wait(semaphore);
        printf(format, baseData[count].filename, baseData[count].pid, baseData[count].md5);
        count++;
    }
}