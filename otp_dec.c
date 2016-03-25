// Michael Bamesberger
// CS344 Program 4
// otp_dec connects to otp_dec_d, asks to decrypt cipher


#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void keyHandler(int sockfd, char fileName[]);
void textHandler(int sockfd, char fileName[]);

int main(int argc, char *argv[]) {
    FILE *text, *key;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int textCount, keyCount;
    int n, portno, sockfd;
    
    if(argc < 2){
        printf("Usage: otp_dec key port\n");
        exit(1);
    }
    
    text = fopen(argv[1], "r");
    key = fopen(argv[2], "r");
    fseek(text, 0, SEEK_END);
    fseek(key, 0, SEEK_END);
    textCount = ftell(text);
    keyCount = ftell(key);
    fclose(text);
    fclose(key);
    
    if(keyCount < textCount) {                          //Check key length
        printf("Error: key is too short\n");
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror( "Error: error opening socket\n" );
        exit(1);
    }
    
    portno = atoi(argv[3]);
    server = gethostbyname("localhost");
    bzero((char *) &serv_addr, sizeof(serv_addr));                   //Initialize server
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); 
    serv_addr.sin_port = htons(portno);
  
    connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));   // connect to server
    char buffer[2];
    bzero(buffer, 2);
    n = recv(sockfd,buffer,2,0);
    
    if(buffer[0] == 'd') {
        textHandler(sockfd, argv[1]);                                   //Send text
        char confirmation[2];
        bzero(confirmation, 2);
        n = recv(sockfd,confirmation,2,0);
        if(confirmation[0] == '1') {                                    //If confirmation received send key
            keyHandler(sockfd, argv[2]);
        }

        char cipherMessage[textCount];
        bzero(cipherMessage, textCount);
        int m = recv(sockfd,cipherMessage,textCount - 1,0);
        sprintf(cipherMessage, "%s\0", cipherMessage);
        printf("%s", cipherMessage);
        printf("\n");
    }
    else {
        printf("Couldn't connect to otp_dec_d\n");
        exit(2);
    }
    
    close(sockfd);
    return 0;
}

void textHandler(int sockfd, char fileName[]) {
    char buffer[10];
    FILE *fs = fopen(fileName, "r");
    bzero(buffer, 10);
    int fsSize, bytes;
    while((fsSize = fread(buffer, sizeof(char), 10, fs)) > 0) {
        if((bytes = send(sockfd, buffer, fsSize, 0)) < 0) {
            break;
        }
        bzero(buffer, 10);
    }
    if(bytes == 10) {
        send(sockfd, "0", 1, 0);                            // end server's transmission
    }
    fclose(fs);
    return;
}

void keyHandler(int sockfd, char fileName[]) {
    char buffer[10];
    FILE *fs = fopen(fileName, "r");
    bzero(buffer, 10);
    int fsSize, bytes;
    while((fsSize = fread(buffer, sizeof(char), 10, fs)) > 0) {
        if((bytes = send(sockfd, buffer, fsSize, 0)) < 0) {
            break;
        }
        bzero(buffer, 10);
    }
    if(bytes == 10) {
        send(sockfd, "0", 1, 0);     }
    fclose(fs);
    return;
}

