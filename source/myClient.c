#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define SMALL_BUFFER_SIZE 128
#define PORT_CC 34324

#define SHUTDOWN_FROM_CLIENT "SHUTDOWN"
#define RECOGNIZE_PASSWORD "Enter password"
#define RECOGNIZE_DOWNLOAD "DOWNLOAD :"

#define MAX_BOOK_NAME_LENGTH 100

void removeNewLine(char *str){
    int newLinePos = strcspn(str,"\n");
    str[newLinePos] = '\0';
}

void clearBuffer(char buffer[BUFFER_SIZE]){
    memset(buffer,0,BUFFER_SIZE);
}

int receiveFile(int clientFd, const char *filename) {

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file == -1) {
        perror("open");
        close(clientFd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;

    //what i recv() i write() in binary in the specified path
    while ((bytesReceived = recv(clientFd, buffer, sizeof(buffer), 0)) > 0) {
        if (write(file, buffer, bytesReceived) == -1) {
            perror("error reconstructing the file in client");
            close(file);
            close(clientFd);
            return -1;
        }

        if (strstr(buffer, "FILE_TRANSFER_COMPLETE") != NULL) {
        printf("File transfer complete.\n");
        break;
        }
    }

    printf("'%s' successfully downloaded \n", filename);
    close(file);
    return 1;
}

int main(){

    char receivingBuffer[BUFFER_SIZE];
    char baseFilename[BUFFER_SIZE] = "downloaded";
    char filename[BUFFER_SIZE];
    strcpy(filename, baseFilename);

    int networkSocket = socket(AF_INET,SOCK_STREAM,0);
    if(networkSocket == -1){
        perror("socket client");
        exit(EXIT_FAILURE);
    }

    char sendingBuffer[BUFFER_SIZE];
    
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT_CC);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(connect(networkSocket,(struct sockaddr*) &serverAddress,sizeof(serverAddress)) == -1){
        perror("connect");
        exit(1);
    }

    bool shouldDownload = false;
    bool running = true;

    while(running){

        shouldDownload = false;
        ssize_t bytesReceived = recv(networkSocket,receivingBuffer,BUFFER_SIZE,0);
        printf("(CLIENT) got '%s' \n",receivingBuffer);
        
        fgets(sendingBuffer, BUFFER_SIZE, stdin);
        removeNewLine(sendingBuffer);

        if(strncmp(sendingBuffer, RECOGNIZE_DOWNLOAD,strlen(RECOGNIZE_DOWNLOAD)) == 0){
            shouldDownload = true;
            char* bookInfo = sendingBuffer + strlen(RECOGNIZE_DOWNLOAD);
            char* title = strtok(bookInfo, ",");
            char* author = strtok(NULL, ",");
            
            if(title != NULL && author != NULL){
                strcat(filename, title);
                strcat(filename, ".pdf");
                printf("Filename = '%s' \n",filename);
            }
        }

        if(strcmp(sendingBuffer,"SHUTDOWN") == 0){
            send(networkSocket,SHUTDOWN_FROM_CLIENT,sizeof(sendingBuffer),0);
            running = false;
        } else {
            int bytesSent = send(networkSocket,sendingBuffer,sizeof(sendingBuffer), 0);
            if (bytesSent < 0){
                perror("send client");
                running = false;
                break;
            }
        }

        if(shouldDownload){
            int receiveStatus = receiveFile(networkSocket,filename);
            clearBuffer(filename);
            strcpy(filename, baseFilename);
        } else {
            recv(networkSocket,receivingBuffer,BUFFER_SIZE,0);
            printf("(CLIENT) got '%s' \n", receivingBuffer);
            clearBuffer(receivingBuffer);
        }
        
    }

    close(networkSocket);
}



