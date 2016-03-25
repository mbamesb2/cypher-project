// Michael Bamesberger
// CS344 Program 4
// otp_enc_d runs in the background, performs encoding, writes back ciphertext

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>


void processing(int socketfd, int pid);
void keyHandler(int socketfd, int pid);
int fileHandler(int socketfd, int pid);
void encoder(int pid, int num, int socketdfd);

int main(int argc, char *argv[]) {
    
    //variable definitions
    int sockfd, newsockfd, portno, pid;
    socklen_t client_length;
    struct sockaddr_in server_addr, client_addr;
    
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                       //Create socket
    if( sockfd < 0 )
    {
        perror( "Error: Error opening socket\n" );
        exit(1);
    }
    
    bzero((char *) &server_addr, sizeof(server_addr));                  //initialize server adress
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);
    
    
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){       //bind to host address
        perror( "Error: Error binding\n" );
        exit(1);
    };
    
    
    listen(sockfd, 5);
    client_length = sizeof(client_addr);
    
    while(1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_length); //Accept all sockets
        if(newsockfd < 0)
        {
            perror("Error: Error on accept");
            exit(1);
        }

        pid = fork();                                                           //Creating a child
        if(pid == 0) {
            close(sockfd);
            int cpid = getpid();
            processing(newsockfd, cpid);                             //Call the chat function
            exit(0);
        }
    }
    return 0;
}

int fileHandler(int socketfd, int pid) {
    FILE *text;
    char fr_name[10];
    int write_sz;
    bzero(fr_name, 10);
    char receivedMessage[10];
    sprintf(fr_name, "%dt", pid);
    FILE *fr = fopen(fr_name, "a");
    int fr_block_sz = 0;
    while((fr_block_sz = recv(socketfd, receivedMessage, 10, 0)) > 0) {
        write_sz = fwrite(receivedMessage, sizeof(char), fr_block_sz, fr);    //Write to the file
        if(write_sz < fr_block_sz) {
            //printf("error");
        }
        bzero(receivedMessage, 10);
        if (fr_block_sz == 0 || fr_block_sz != 10){
            break;
        }
    }
    
    fclose(fr);
    text = fopen(fr_name, "r");
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
            //printf("error");
        }
        bzero(receivedMessage, 10);
        if (fr_block_sz == 0 || fr_block_sz != 10) {
            break;
        }
    }
    fclose(fr);
    return;
}


void encoder(int processId, int numChars, int socket) {
    char letters[28];
    int j = 0;
    FILE *fg;
    char kr_name[10];
    char ciphertext[numChars];
    char fileArray[numChars - 1];                                   //account for newline
    char keyArray[numChars - 1];
    char encodeArr[numChars - 1];
    int i = 0;
    int c, d, remainder;
    
    char fr_name[10];
    bzero(fr_name, 10);
    sprintf(fr_name, "%dt", processId);
    
    FILE *fr = fopen(fr_name, "r");
    while((c = fgetc(fr)) != EOF && i < numChars - 1) {
        fileArray[i] = c;
        i++;
    }
    fclose(fr);
    
    bzero(kr_name, 10);
    sprintf(kr_name, "%dk", processId);
    
   
    fg = fopen(kr_name, "r");
    while((d = fgetc(fg)) != EOF && j < numChars - 1) {             //Fill array
        keyArray[j] = d;
        j++;
    }
    fclose(fg);
    sprintf(letters, "ABCDEFGHIJKLMNOPQRSTUVWXYZ ");
    
    for(i = 0; i < numChars - 1; i++) {
        int sum = 0;
        for(j = 0; j < 28; j++) {
            if(fileArray[i] == letters[j]) {
                sum = sum + j + 1;
            }
            if(keyArray[i] == letters[j]) {
                sum = sum + j + 1;
            }
        }
        remainder = sum % 27;
        encodeArr[i] = letters[remainder];
    }
    
    bzero(ciphertext, numChars);
    sprintf(encodeArr, "%s\0", encodeArr);
    int n = write(socket,encodeArr,numChars - 1);
    unlink(fr_name);
    unlink(kr_name);
    return;
}

void processing(int socketfd, int pid){
    int writevar, numChars;
    char buffer[2];
    char confirm[2];
    bzero(buffer, 2);
    sprintf(buffer, "e");
    writevar = write(socketfd,buffer,strlen(buffer));
    numChars = fileHandler(socketfd, pid);
    bzero(confirm,1);
    sprintf(confirm, "1");
    writevar = write(socketfd,confirm,strlen(confirm));
    keyHandler(socketfd, pid);                                                  //Get keyfile
    encoder(pid, numChars, socketfd);
    close(socketfd);
    return;
}
