#include "../include/md5Lib.h"


int createChildsAndPipes (int childsQty,int  md5SendData[][2],int  slaveSendData[][2],int * pids){
   
    pid_t pidAux;
    char *const argvC[] = { "./slave", NULL };
    
    for (int i = 0; i < childsQty; i++ ) {

        if (pipe(md5SendData[i]) == -1 || pipe(slaveSendData[i]) == -1) {
            return ERROR;
        }
	    pidAux = fork();
        if (pidAux == 0) {

            close(md5SendData[i][W_END]); 
            close(slaveSendData[i][R_END]);
            dup2(md5SendData[i][R_END],STDIN_FILENO);
            dup2(slaveSendData[i][W_END],STDOUT_FILENO);

	        close(md5SendData[i][R_END]);
	        close(slaveSendData[i][W_END]);

	        for (int k=0; k < i;k++) {
                close(md5SendData[k][W_END]);
                close(md5SendData[k][R_END]);
                close(slaveSendData[k][R_END]);
	            close(slaveSendData[k][W_END]);
            }
            execve("./slave", argvC, NULL);
            return ERROR;
        }
        else {
		    pids[i] = pidAux;
        }
    }

    for (int i = 0; i < childsQty; i++)
    {
        close(md5SendData[i][R_END]);
        close(slaveSendData[i][W_END]);
    }
    return 0;
}

void sendData(int fd, const char *message[], int *dataLeft, int *idx, int qty) {
    for (int i = 0; i < qty && (*dataLeft) > 0; i++) {
        char tmpBuffer[MAXFILELEN]={'\0'}; 
        snprintf(tmpBuffer, sizeof(tmpBuffer), "%s\n", message[i]);
        write(fd, tmpBuffer, MAXFILELEN);
        (*dataLeft)--;
        (*idx)++;
    }
}

void listenChilds(fd_set* read_fds,int slaveSendData[][2],int childsQty){
    FD_ZERO(read_fds);
        for (int i = 0; i < childsQty; i++)
        {
             FD_SET(slaveSendData[i][0],read_fds);
        }
    select(slaveSendData[childsQty-1][0]+1,read_fds,NULL,NULL,NULL);
}

void processChild(fd_set* read_fds,int slaveSendData,int sizeBufferPipe,pid_t pid,int* idxOut,message* shmPtr,sem_t * semAddress,int fdOut){
            if(FD_ISSET(slaveSendData,read_fds)) {
                char* token;
                char bufferPipe[sizeBufferPipe];
                int nullTerminated = read(slaveSendData,bufferPipe,sizeBufferPipe);
                
                bufferPipe[nullTerminated] = '\0';
                token = strtok(bufferPipe," \n");
                while (token != NULL)
                {      
                    strcpy(shmPtr[(*idxOut)].md5,token);
                    token = strtok(NULL," \n");                    
                    if(token == NULL){
                        ERRORMSG("Wrong md5sum format");
                    }
                    strncpy(shmPtr[(*idxOut)].filename,token, MAXFILELEN);
                    shmPtr[(*idxOut)].pid = pid;
                    char tmpBuffer[sizeof("Filename: %s - PID: %d - MD5: %s\n") + MD5LEN + MAXFILELEN + 2]={'\0'};
                    snprintf(tmpBuffer, sizeof(tmpBuffer),"Filename: %s - PID: %d - MD5: %s\n", shmPtr[*idxOut].filename, pid, shmPtr[*idxOut].md5);
                    sem_post(semAddress);
                    write(fdOut,tmpBuffer,strlen(tmpBuffer));
                    (*idxOut)++;
                    token = strtok(NULL," \n");
                }
            }
}


void terminateChildren(int qty,int md5SendData[][2],int slaveSendData[][2], pid_t pids[]){
    
    for (int j=0;j< qty; j++) {
        close(md5SendData[j][1]);
        close(slaveSendData[j][0]);
        waitpid(pids[j],NULL,0);
    }
}