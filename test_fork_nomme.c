#include "conduct.h"
#include <sys/wait.h>

int main(int argc, char const *argv[]) {

    pid_t pid;

    pid = fork();
    if(pid < 0){
        perror("fork failed");
        exit(1);
    } else if(pid == 0){

        struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
        if(serveur == NULL){
            perror("serveur null");
        }
        char* buff = malloc(sizeof(char)*4);
        buff = "test";
        conduct_write(serveur, buff, sizeof(buff));
        conduct_close(serveur);
        exit(0);
    } else {
        struct conduct* client = conduct_open("serveur");
        char* reponse = malloc(sizeof(char)*4);
        wait(NULL);
        conduct_read(client, reponse, sizeof(reponse));
        write(1,reponse,strlen(reponse));
        write(1, "\n", 1);
        conduct_destroy(client);
        exit(EXIT_SUCCESS);
    }

}
