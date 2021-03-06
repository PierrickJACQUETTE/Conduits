#include "conduct.h"
#include <sys/wait.h>

int main(int argc, char const *argv[]) {

    pid_t pid;
    int size = 10;

    pid = fork();
    if(pid < 0){
        perror("fork failed");
        exit(1);
    } else if(pid == 0){

        struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
        if(serveur == NULL){
            perror("serveur null");
        }
        char* buff = malloc(sizeof(char)*size);
        memset(buff, 0, size+1);
        buff = "azertyuioz";
        conduct_write(serveur, buff, strlen(buff));
        buff = "azrtpoiupa";
        conduct_write(serveur, buff, strlen(buff));
        conduct_close(serveur);
        exit(0);
    } else {
        struct conduct* client = conduct_open("serveur");
        char* reponse = malloc(sizeof(char)*size);
        memset(reponse, 0, size+1);
        conduct_read(client, reponse, size);
        write(1,reponse,strlen(reponse));
        write(1, "\n", 1);
        conduct_read(client, reponse, size);
        write(1,reponse,strlen(reponse));
        write(1, "\n", 1);
        conduct_destroy(client);
        exit(EXIT_SUCCESS);
    }

}
