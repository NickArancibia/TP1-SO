#ifndef __MD5LIB_H
#define __MD5LIB_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "sharedMemory.h"
#include "vistaLib.h"
#define ERROR -1
#define R_END 0
#define W_END 1

int createChildsAndPipes(int childsQty,int  md5SendData[][2],int slaveSendData[][2],int * pids);
void sendData(int fd,char const *message[],int* dataLeft,int* idx,int qty, int dataSize);
void listenChilds(fd_set* read_fds,int slaveSendData[][2],int childsQty);
void processChild(fd_set* read_fds,int slaveSendData,int sizeBufferPipe,pid_t pid,int* idxOut,message* shmPtr,sem_t * semAddress,int fdOut);
void terminateChildren(int qty,int md5SendData[][2],int slaveSendData[][2], pid_t pids[]);
#endif