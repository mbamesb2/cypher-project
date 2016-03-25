// Michael Bamesberger
// CS344 Program 4
// keygen creates a keyfile using randomization methods

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define MAX_LENGTH 160000

int main(int argc, char* argv[]){
    
    //variable definitions
    srand(time(NULL));
    int keyLength;
    char *endptr;
    
    if((argc != 2 && argc != 4)){                            // if incorrect number of arguments
        fprintf(stderr, "Usage: keygen length or keygen length > filename\n");
        exit(1);
    }
 
    errno = 0;
    keyLength = strtol(argv[1], &endptr, 10);               // converts input string to int
    
    if ((errno == ERANGE && (keyLength == INT_MAX || keyLength == INT_MAX)) || (errno != 0 && keyLength == 0)) {
        perror("strtol");
        fflush(stdout);
        exit(1);
    }
    
    else if(!keyLength){
        fprintf(stderr, "Error: must be an int\n");
        fflush(stdout);
        exit(1);
    }
    
    char key[MAX_LENGTH] = {0};
    char charsAllowed[27] = {-1};
    charsAllowed[26] = 32;
    int	i;
    for(i = 0; i < 26; i++){
        charsAllowed[i] = i + 65;
    }
    for(i = 0; i < keyLength; i++){
        key[i] = charsAllowed[rand()%27];         // fills array with random chars
    }
    printf("%s\n", key);
    fflush(stdout);
    return 0;
}