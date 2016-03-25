// Michael Bamesberger
// CS344 Program 4
// otp_enc_d runs in the background, performs decryption, returns plaintext


#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void processing(int socket, int processId);
int fileHandler(int socket, int processId);
void dencoder(int processId, int numChars, int socket);
void keyHandler(int socket, int processId);

int main(int argc, char *argv[]) {
    
    //variable definitions
    int cpid;
    int sockfd, newsocket, portno, processId;
    socklen_t clientLength;
    struct sockaddr_in server_addr, client_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                               //create socket
    bzero((char *) &server_addr, sizeof(server_addr));
    portno = atoi(argv[1]);
    
    
    server_addr.sin_family = AF_INET;                                       //server init
    server_addr.sin_port = htons(portno);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("Error: error on binding");
        exit(1);
    }
    
    listen(sockfd, 5);
    clientLength = sizeof(client_addr);
    
    while(1) {
        newsocket = accept(sockfd, (struct sockaddr *) &client_addr, &clientLength);
        if( newsocket < 0 )
        {
            perror("Error: error on accept" );
            exit(1);
        }
        
        processId = fork();                             //create child
        if(processId == 0) {
            close(sockfd);
            cpid = getpid();
            processing(newsocket, cpid); //Call the chat function on the socket
            exit(0);
        }
    }
    return 0;
}



int fileHandler(int socket, int processId) {
    char fr_name[10];
    bzero(fr_name, 10);
    char receivedMessage[10];
    sprintf(fr_name, "%dt", processId);
    FILE *fr = fopen(fr_name, "a");
    int fr_block_sz = 0;
    while((fr_block_sz = recv(socket, receivedMessage, 10, 0)) > 0) {
        int write_sz = fwrite(receivedMessage, sizeof(char), fr_block_sz, fr);
        if(write_sz < fr_block_sz) {
            //printf("Error.\n");
        }
        bzero(receivedMessage, 10);
        if (fr_block_sz == 0 || fr_block_sz != 10) {
            break;
        }
    }
    fclose(fr);
    FILE *text = fopen(fr_name, "r");
    fseek(text, 0, SEEK_END);
    int numChars = ftell(text);
    return numChars;
}

void keyHandler(int socket, int processId) {
    char fr_name[10];
    bzero(fr_name, 10);
    char receivedMessage[10];
    sprintf(fr_name, "%dk", processId);
    FILE *fr = fopen(fr_name, "a");
    int fr_block_sz = 0;
    while((fr_block_sz = recv(socket, receivedMessage, 10, 0)) > 0) {
        int write_sz = fwrite(receivedMessage, sizeof(char), fr_block_sz, fr);
        if(write_sz < fr_block_sz) {
            //printf("Error.\n");
        }
        bzero(receivedMessage, 10);
        if (fr_block_sz == 0 || fr_block_sz != 10) {
            break;
        }
    }
    fclose(fr);
    return;
}

void dencoder(int processId, int numChars, int socket) {
    char letterArrary[28];
    char plaintext[numChars];
    char fileArray[numChars - 1];
    char keyArray[numChars - 1];
    char decArray[numChars - 1];
    int i = 0;
    int c;
    
    char fr_name[10];
    bzero(fr_name, 10);
    sprintf(fr_name, "%dt", processId);
    
    FILE *fr = fopen(fr_name, "r");
    while((c = fgetc(fr)) != EOF && i < numChars - 1) {
        fileArray[i] = c;
        i++;
    }
    fclose(fr);
    
    
    char kr_name[10];
    bzero(kr_name, 10);
    sprintf(kr_name, "%dk", processId);
    
    int j = 0;
    int d;
    FILE *kr = fopen(kr_name, "r");
    while((d = fgetc(kr)) != EOF && j < numChars - 1) {
        keyArray[j] = d;
        j++;
    }
    fclose(kr);
    
    sprintf(letterArrary, " ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    
    for(i = 0; i < numChars - 1; i++) {
        int sum = 0;
        for(j = 0; j < 28; j++) {
            if(fileArray[i] == letterArrary[j]) {
                sum = sum + j;
            }
            if(keyArray[i] == letterArrary[j]) {
                sum = sum - j;
            }
        }
        sum = sum - 1;
        if(sum < 0) {
            sum = sum + 27;
        }
        decArray[i] = letterArrary[sum];
    }
    
    bzero(plaintext, numChars);
    sprintf(decArray, "%s\0", decArray);
    int n = write(socket,decArray,numChars - 1);
    unlink(fr_name);
    unlink(kr_name);
    return;
}

void processing(int socket, int processId) {
    char confirmation[2];
    int n;
    char type[2];
    bzero(type, 2);
    sprintf(type, "d");
    n = write(socket,type,strlen(type));
    int numChars = fileHandler(socket, processId);
    bzero(confirmation,1);
    sprintf(confirmation, "1");
    n = write(socket,confirmation,strlen(confirmation));
    keyHandler(socket, processId);
    dencoder(processId, numChars, socket);
    close(socket);
    return;
}

