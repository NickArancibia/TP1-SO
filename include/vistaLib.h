#ifndef __VISTALIB_H
#define __VISTALIB_H

#include "./sharedMemory.h"
#include <string.h>

#define MAXNAMELEN 50
#define ERRORMSG(msg) { perror(msg); exit(1); }

void getMemPath(char * memName, int argc, const char * argv1);
void getDataToRead(int * dataLeft, int memoryFd);
void readAndCopyData(int dataLeft, int memoryFd, sem_t * semaphore, message * baseData);

#endif