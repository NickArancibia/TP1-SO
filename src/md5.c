#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "../include/sharedMemory.h"
#include "../include/vistaLib.h"
#define CHILDS_QTY 4
#define SHM_NAME "sharedMemory"
#define INTIAL_LOAD 2 
#include "../include/md5Lib.h"


void freeResources(char* bufferPipe,sem_t* semAddress,char* shmName,void* shmPtr,int shmsize,int shmFd);
void listenChilds(fd_set* read_fds,int slaveSendData[][2],int childsQty);

int main(int argc, char const *argv[])
{
    int filesQty = argc - 1;
    int shmSize = sizeof(argc) + sizeof(sem_t) + sizeof(message) * filesQty;
    int shmFd,index =1,idxOut=0;
    int childsQty = (filesQty > CHILDS_QTY)? CHILDS_QTY:filesQty;

    void * shm_ptrBase = createSharedMemory(SHM_NAME,shmSize,&shmFd);
    *((int *)(shm_ptrBase)) = filesQty;
    sem_t *semAddress = (sem_t *)shm_ptrBase + sizeof(argc);
    message* output = (message *)(shm_ptrBase + sizeof(sem_t) + sizeof(filesQty));

    int sizeBufferPipe = (MAXFILELEN+MD5LEN+4)*filesQty;
    char* bufferPipe = calloc(INTIAL_LOAD,sizeBufferPipe);

    int md5SendData[CHILDS_QTY][2],slaveSendData[CHILDS_QTY][2];
    pid_t pids[CHILDS_QTY];

    if (sem_init(semAddress, 1, 0) == -1) {
        free(bufferPipe);
        destroySharedMemory(SHM_NAME,shm_ptrBase,shmFd,shmSize);
        ERRORMSG("sem_init")
    }

    write(STDOUT_FILENO, SHM_NAME, strlen(SHM_NAME));
    sleep(3);
    createChildsAndPipes(childsQty,md5SendData,slaveSendData,pids);
 

    int fdResults = open("results.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fdResults == -1) {
        free(bufferPipe);
        sem_destroy(semAddress);
        destroySharedMemory(SHM_NAME,shm_ptrBase,shmFd,shmSize);
        ERRORMSG("Error opening results.txt");
    }


    for(int i = 0; i<childsQty; i++) {
        sendData(md5SendData[i][1],(argv+index),&filesQty,&index,INTIAL_LOAD,MAXFILELEN+1);
    }

    fd_set read_fds;
    while (idxOut < (argc-1)) {

	    listenChilds(&read_fds,slaveSendData,childsQty);
        
        for(int i=0; i < childsQty ;i++) {
            if(FD_ISSET(slaveSendData[i][0],&read_fds)) {
		        char* token;
                int nullTerminated = read(slaveSendData[i][0],bufferPipe,sizeBufferPipe);
                if(nullTerminated == -1){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                bufferPipe[nullTerminated] = '\0';
                token = strtok(bufferPipe," \n");
                while (token != NULL)
                {      
                    strcpy(output[idxOut].md5,token);
                    token = strtok(NULL," \n");                    
                    strncpy(output[idxOut].filename,token, MAXFILELEN);
                    output[idxOut].pid = pids[i];
                    char tmpBuffer[sizeof("Filename: %s - PID: %d - MD5: %s\n") + MD5LEN + MAXFILELEN + 2]={'\0'};
                    snprintf(tmpBuffer, sizeof(tmpBuffer),"Filename: %s - PID: %d - MD5: %s\n", output[idxOut].filename, pids[i], output[idxOut].md5);
                    sem_post(semAddress);
                    write(fdResults,tmpBuffer,strlen(tmpBuffer));
                    idxOut++;
                    token = strtok(NULL," \n");
                }
                
                 if(filesQty >0){
                   sendData(md5SendData[i][1],(argv+index),&filesQty,&index,1,MAXFILELEN+1);
                }
            }
        }

    }





    sem_close(semAddress);
    for (int j=0;j< childsQty; j++) {
        close(md5SendData[j][1]);
        close(slaveSendData[j][0]);
    }
    for (int j=0;j< childsQty; j++) {
        waitpid(pids[j],NULL,0);
    }
    close(fdResults);
    freeResources(bufferPipe,semAddress,SHM_NAME,shm_ptrBase,shmSize,shmFd);
    write(STDOUT_FILENO, "\n", 1);

    return 0;
}



void listenChilds(fd_set* read_fds,int slaveSendData[][2],int childsQty){
    FD_ZERO(read_fds);
        for (int i = 0; i < childsQty; i++)
        {
             FD_SET(slaveSendData[i][0],read_fds);
        }
    select(slaveSendData[childsQty-1][0]+1,read_fds,NULL,NULL,NULL);
}


void freeResources(char* bufferPipe,sem_t* semAddress,char* shmName,void* shmPtr,int shmSize,int shmFd){
    if(semAddress != NULL){
        sem_close(semAddress);
        sem_destroy(semAddress);
    }
    if(bufferPipe != NULL){
        free(bufferPipe);
    }
    destroySharedMemory(shmName,shmPtr,shmFd,shmSize);
    
}
