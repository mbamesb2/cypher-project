// Michael Bamesberger
// CS344 Program 4
// otp_enc connects to opt_enc_d, asks it to perform one-time pad style encryption


#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void textHandler(int sockfd, char fileName[]);
void keyHandler(int sockfd, char fileName[]);


int main(int argc, char *argv[]) {
    
    //variable definitions
    FILE *text, *key, *fr;
    int textCount, keyCount;
    char c;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char shake[2];
    
    text = fopen(argv[1], "r");
    key = fopen(argv[2], "r");
    fseek(text, 0, SEEK_END);
    fseek(key, 0, SEEK_END);
    textCount = ftell(text);
    keyCount = ftell(key);
    fclose(text);
    fclose(key);
    
    if (argc < 4) {
        fprintf(stderr,"usage %s plaintext key port#\n", argv[0]);
        exit(1);
    }
    
    if(keyCount < textCount) {                          //Check key length
        printf("Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }
    
    fr = fopen(argv[1], "r");                           //Check bad characters
    while((c = fgetc(fr)) != EOF) {
        if(c == '$') {
            fprintf(stderr, "Error: %s contains bad characters\n", argv[1]);
            exit(1);
        }
    }
    
    fclose(fr);


    sockfd = socket(AF_INET, SOCK_STREAM, 0);               //New socket
    if (sockfd < 0){
        printf("Error: socket did not open\n");
        exit(1);
    }
    
    portno = atoi(argv[3]); 
    server = gethostbyname("localhost");
    if (server == NULL){
        printf("Error: no such host\n");
        exit(1);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
   
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){       //Connect to server
        printf("Error: connection error\n");
        exit(1);
        
    };
    
    
    bzero(shake, 2);
    n = recv(sockfd,shake,2,0);
    if(shake[0] == 'e') {
        textHandler(sockfd, argv[1]);
        char confirmation[2];
        bzero(confirmation, 2);
        n = recv(sockfd,confirmation,2,0);
        if(confirmation[0] == '1') {
            keyHandler(sockfd, argv[2]);                                        //Send the key file
        }
        //Get the encoded message back
        char encodedMessage[textCount];
        bzero(encodedMessage, textCount);
        int m = recv(sockfd,encodedMessage,textCount - 1,0);
        sprintf(encodedMessage, "%s\0", encodedMessage);
        printf("%s", encodedMessage);
        printf("\n");
    }
    
    else {
        printf("Could not connect to otp_enc_d\n");
        exit(2);
    }
    
    close(sockfd);
    return 0;
}

void textHandler(int sockfd, char fileName[]) {                             //sends txt to server
    char buffer[10];
    FILE *fs = fopen(fileName, "r");
    bzero(buffer, 10);
    int fs_block_sz;
    int bytesSent = -1;
    while((fs_block_sz = fread(buffer, sizeof(char), 10, fs)) > 0) {
        if((bytesSent = send(sockfd, buffer, fs_block_sz, 0)) < 0) {
            break;
        }
        bzero(buffer, 10);
    }
    if(bytesSent == 10) {                                                   //Check if sent 10 bytes to end transmission
        send(sockfd, "0", 1, 0);
    }
    fclose(fs);
    return;
}

void keyHandler(int sockfd, char fileName[]){                               //send key
    char buffer[10];
    FILE *fs = fopen(fileName, "r");
    bzero(buffer, 10);
    int fs_block_sz;
    int bytesSent;
    while((fs_block_sz = fread(buffer, sizeof(char), 10, fs)) > 0) {
        if((bytesSent = send(sockfd, buffer, fs_block_sz, 0)) < 0) {
            break;
        }
        bzero(buffer, 10);
    }
    if(bytesSent == 10) {
        send(sockfd, "0", 1, 0);
    }
    fclose(fs);
    return;
}

