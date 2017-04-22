#include "conduct.h"

int main(int argc, char const *argv[]) {
    struct conduct* client = conduct_open("serveur");
    if(client == NULL){
        perror("client null");
    }
    char buff[30];
    int nbEcrit, i;
    for(i=1; i<argc; i++){
        strncpy(buff, argv[i], 29);
		buff[30] = '\0';
        nbEcrit = conduct_write(client, buff, 30);
        if(nbEcrit < 0){
            perror("conduct_write");
        }
    }

    // char* buff = malloc(sizeof(char)*4);
    // buff = "test";
    // conduct_write(client, buff, sizeof(buff));
    // printf("done writing\n");


    conduct_close(client);
    exit(EXIT_SUCCESS);
}
