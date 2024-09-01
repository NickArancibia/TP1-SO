#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
//#include <definitions.h>
#define MAXFILELEN 100

void createChild(char *str);

int main(int argc, char *argv[]){

    if(argc > 1){
        fprintf(stderr,"Argument error: No arguments accepted\n");
        return EXIT_FAILURE;
    }

    char buffer[MAXFILELEN];
    int bytesread;

    while((bytesread = read(STDIN_FILENO, buffer, MAXFILELEN)) != 0){
        char *lineStart;
        char *lineEnd;
        lineStart = buffer;

        while((lineEnd = strchr(lineStart, ' ')) != NULL || (lineEnd = strchr(lineStart, '\n')) != NULL){
            *lineEnd = '\0';
            createChild(lineStart);
            lineStart = lineEnd + 1;
        }
    }
}

void createChild(char *str){
    pid_t pid;
    if( (pid = fork()) == -1){
        fprintf(stderr,"Fork error: failed to create a child\n");
        return;
    }

    if(pid == 0){
        char *argv[] = {"/usr/bin/md5sum", str, NULL};
        execve("/usr/bin/md5sum", argv, NULL);

        fprintf(stderr,"Execve error\n"); //TODO: chequear si esta bien el mensaje de error.
        return;
    }
    wait(NULL);
}