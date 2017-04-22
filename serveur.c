#include "conduct.h"
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
    struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
    if(serveur == NULL){
        perror("serveur null");
    }
    int nbLu, erreur;
    char buff[50];
    while(1){
        nbLu = conduct_read(serveur, buff, sizeof(buff));
        if(nbLu < 0){
            perror("conduct_read");
        }
        else if(nbLu > 0){
            erreur = write(STDOUT_FILENO, "message : ", 10);
            if(erreur == -1){
                perror("write");
            }
            erreur = write(STDOUT_FILENO, buff, nbLu);
            if(erreur == -1){
                perror("write");
            }
        }
    }

    sleep(30);

}
