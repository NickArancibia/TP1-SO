#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "../include/sharedMemory.h"
#include "../include/vistaLib.h"
#include "../include/md5Lib.h"

#define SHM_NAME "sharedMemory"
#define CHILDS_QTY 5
#define INITIAL_LOAD_PERCENTAGE 0.1 
#define DEFAULT_INITIAL_LOAD 1
#define MIN_FILES_PER_SON 20
void freeResources(char* bufferPipe,sem_t* semAddress,char* shmName,void* shmPtr,int shmsize,int shmFd);

int main(int argc, char const *argv[])
{
    int filesQty = argc - 1;
    int shmSize = sizeof(argc) + sizeof(message) * filesQty;
    int shmFd,index =1,dataRead=0;
    int childsQty = (filesQty > (CHILDS_QTY * MIN_FILES_PER_SON))? CHILDS_QTY: (int)(filesQty/(MIN_FILES_PER_SON));
    if(childsQty == 0){ childsQty = 1;}
    int initialLoad = ((int)((filesQty/childsQty)*INITIAL_LOAD_PERCENTAGE )> 0) ? (filesQty/childsQty)*INITIAL_LOAD_PERCENTAGE : DEFAULT_INITIAL_LOAD;

    void * shm_ptrBase = createSharedMemory(SHM_NAME,shmSize,&shmFd);
    *((int *)(shm_ptrBase)) = filesQty;
    message* output = (message *)(shm_ptrBase + sizeof(filesQty));

    sem_t * semAddress = sem_open(SHM_NAME, O_CREAT, 0666, 0);

    if (semAddress == SEM_FAILED) {
        freeResources(NULL,NULL,SHM_NAME,shm_ptrBase,shmSize,shmFd);
        ERRORMSG("sem_init")
    }

    int sizeBufferPipe = (MAXFILELEN+MD5LEN+4)*initialLoad;
    char* bufferPipe = calloc(1,sizeBufferPipe);

    if(bufferPipe == NULL){
        freeResources(NULL,semAddress,SHM_NAME,shm_ptrBase,shmSize,shmFd);
        ERRORMSG("Error allocating memory");
    }

    int md5SendData[CHILDS_QTY][2],slaveSendData[CHILDS_QTY][2];
    pid_t pids[CHILDS_QTY];
    
    write(STDOUT_FILENO, SHM_NAME, strlen(SHM_NAME));
    sleep(3);
    
    if(createChildsAndPipes(childsQty,md5SendData,slaveSendData,pids) !=0){
        freeResources(bufferPipe,semAddress,SHM_NAME,shm_ptrBase,shmSize,shmFd);
        ERRORMSG("Error creating childs and pipes");
    }
 
    int fdResults = open("results.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fdResults == -1) {
        freeResources(bufferPipe,semAddress,SHM_NAME,shm_ptrBase,shmSize,shmFd);
        ERRORMSG("Error opening results.txt");
    }

    for(int i = 0; i<childsQty; i++) {
        sendData(md5SendData[i][W_END],(argv+index),&filesQty,&index,initialLoad);
    }

    fd_set read_fds;
    while (dataRead < (argc-1)) {
       
	    listenChilds(&read_fds,slaveSendData,childsQty);
        for(int i=0; i < childsQty ;i++) {
            if(FD_ISSET(slaveSendData[i][R_END],&read_fds)) {
		        processChild(&read_fds,slaveSendData[i][R_END],sizeBufferPipe,pids[i],&dataRead,output,semAddress,fdResults);
                 if(filesQty >0){
                   sendData(md5SendData[i][W_END],(argv+index),&filesQty,&index,1);
                }
            }
        }
    }
    terminateChildren(childsQty,md5SendData,slaveSendData,pids);
    close(fdResults);
    freeResources(bufferPipe,semAddress,SHM_NAME,shm_ptrBase,shmSize,shmFd);
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}

void freeResources(char* bufferPipe,sem_t* semAddress,char* shmName,void* shmPtr,int shmSize,int shmFd){
    if(semAddress != NULL){
        sem_close(semAddress);
        sem_destroy(semAddress);
        sem_unlink(shmName);
    }
    if(bufferPipe != NULL){
        free(bufferPipe);
    }
    destroySharedMemory(shmName,shmPtr,shmFd,shmSize);
}
