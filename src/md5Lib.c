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



void sendData(int fd, const char *message[],int* dataLeft,int* idx,int qty, int dataSize) {

    char* tmpBuffer = calloc(qty,dataSize);
    int sizeTotal = qty*dataSize; 
    int len = 0;
    tmpBuffer[len] = '\0';
    if(*dataLeft > 0){
    for (int i = 0; i < qty && (*dataLeft) > 0 ; i++)
    {
        if (i > 0) {
            len += snprintf(tmpBuffer + len, sizeTotal - len, "%s", " ");
        }
        len += snprintf(tmpBuffer + len, sizeTotal - len, "%s", message[i]);
        (*dataLeft)--;
        (*idx)++;
    }
    
    snprintf(tmpBuffer + len, sizeTotal - len, "\n");
   
    write(fd,tmpBuffer,strlen(tmpBuffer));
    }
    free(tmpBuffer);
}


