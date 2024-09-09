#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "../include/sharedMemory.h"
#define CHILDS_QTY 4
#define SHM_NAME "sharedMemory"
#define INTIAL_LOAD 2 

void sendData(int fd, const char * message);
void * createSharedMemory(char* name,int size, int* fd);

int main(int argc, char const *argv[])
{
    int filesQty = argc - 1;
    int shmSize = sizeof(argc) + sizeof(sem_t) + sizeof(message) * filesQty;
    int shmFd,index =1,idxOut=0;
    void * shm_ptrBase = createSharedMemory(SHM_NAME,shmSize,&shmFd);

    *((int *)(shm_ptrBase)) = filesQty;
    sem_t *semAddress = (sem_t *)shm_ptrBase + sizeof(argc);
    message* output = (message *)(shm_ptrBase + sizeof(sem_t) + sizeof(filesQty));

    if (sem_init(semAddress, 1, 0) == -1) {
        perror("sem_init");
        munmap(shm_ptrBase, shmSize);
        close(shmFd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, SHM_NAME, strlen(SHM_NAME));
    sleep(3);

    char *const argvC[] = { "./slave", NULL };
    int pipesFd[2*CHILDS_QTY][2];
	pid_t pidAux;
    pid_t pids[CHILDS_QTY];

    for (int i = 0; i < CHILDS_QTY; i++ ) {

        if (pipe(pipesFd[2 * i]) == -1 || pipe(pipesFd[2 * i + 1]) == -1) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
	    pidAux = fork();
        if (pidAux == 0) {

            close(pipesFd[2 * i][1]); 
            close(pipesFd[2 * i + 1][0]);

            close(STDIN_FILENO);
	        dup(pipesFd[2*i][0]);
	        close(STDOUT_FILENO);
	        dup(pipesFd[2*i+1][1]);

	        close(pipesFd[2*i][0]);
	        close(pipesFd[2*i+1][1]);

	        for (int j = 0,k=0; k < i;k++, j += 2) {
                close(pipesFd[j][1]);
                close(pipesFd[j][0]);
                close(pipesFd[j+1][0]);
	            close(pipesFd[j+1][1]);
            }
            execve("./slave", argvC, NULL);
            perror("execve failed");
            exit(EXIT_FAILURE);
        }
        else {
		    pids[i] = pidAux;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        close(pipesFd[i*2][0]);
        close(pipesFd[i*2+1][1]);
    }

    int fdResults = open("results.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fdResults == -1) {
        perror("Error opening results.txt");
        exit(EXIT_FAILURE);
    }


    for(int i = 0; i<INTIAL_LOAD; i++) {
        for (int j=0; j<2*CHILDS_QTY; j+=2) {
            if(filesQty >0){
		        sendData(pipesFd[j][1],argv[index++]);
                filesQty--;
            }
        }
    }

    fd_set read_fds;
    while (idxOut < (argc-1)) {

	    FD_ZERO(&read_fds);
        FD_SET(pipesFd[1][0],&read_fds);
        FD_SET(pipesFd[3][0],&read_fds);
        FD_SET(pipesFd[5][0],&read_fds);
        FD_SET(pipesFd[7][0],&read_fds);
        select(pipesFd[7][0]+1,&read_fds,NULL,NULL,NULL);
        for(int j= 1,i=0; i < CHILDS_QTY ;i++,j+=2) {
            if(FD_ISSET(pipesFd[i*2+1][0],&read_fds)) {
		        char entryPipe[MD5LEN+FILENAME_MAX+2]={'\0'};
                int nullTerminated = read(pipesFd[j][0],entryPipe,MD5LEN+FILENAME_MAX);
                if(nullTerminated == -1){
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                entryPipe[nullTerminated] = '\0';
                char * spacePos = strchr(entryPipe, ' ');
                if(spacePos == NULL){
                    perror("strchr");
                    // TODO: Close pipes and unmap
                    exit(EXIT_FAILURE);
                }
                *spacePos = '\0'; 
                char* fileName = spacePos +1;
                strncpy(output[idxOut].md5, entryPipe, MD5LEN);
                strncpy(output[idxOut].filename,fileName, strlen(fileName));
                output[idxOut].pid = pids[i];
		        sem_post(semAddress);
                char buffer[FILENAME_MAX + MD5LEN + 2];
                snprintf(buffer, sizeof(buffer),"Filename: %s - PID: %d - MD5: %s\n", fileName, pids[i], entryPipe);
                write(fdResults,buffer,strlen(buffer));
		        idxOut++;
                if(filesQty >0){
                    sendData(pipesFd[j-1][1],argv[index++]);
                    filesQty--;
                }
            }
        }

    }
    sem_close(semAddress);
    for (int j=0;j< 2* 4; j+=2) {
        close(pipesFd[j][1]);
        close(pipesFd[j+1][0]);
    }
    for (int j=0;j< 4; j++) {
        waitpid(pids[j],NULL,0);
    }
    close(fdResults);
    munmap(shm_ptrBase, shmSize);
    close(shmFd);
    shm_unlink(SHM_NAME);
    write(STDOUT_FILENO, "\n", 1);
    return 0;
}

void sendData(int fd,const char* message) {
    char buffer[MAXFILELEN+2] ={'\0'};
    snprintf(buffer,sizeof(buffer),"%s\n",message);
    write(fd,buffer,strlen(buffer));
}


void * createSharedMemory(char* name,int size, int* fd) {
    *fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (*fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(*fd, size) == -1) {
        perror("ftruncate");
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    void *shm_ptrBase= mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
    if(shm_ptrBase == MAP_FAILED) {
        perror("ftruncate");
        close(*fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    return shm_ptrBase;
}


