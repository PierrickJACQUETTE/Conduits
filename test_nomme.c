#include "conduct.h"

int main(int argc, char const *argv[]) {
    struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
    if(serveur==NULL){
        printf("NULL\n");
    }
    int size = 10;
    char* buff = malloc(sizeof(char)*size);
    memset(buff, 0, size+1);
    buff = "azertyuioz";
    struct conduct* client = conduct_open("serveur");
    conduct_write(serveur, buff, strlen(buff));
    buff = "azrtpoiupa";
    conduct_write(client, buff, strlen(buff));
    char* reponse = malloc(sizeof(char)*size);
    memset(reponse, 0, size+1);
    conduct_read(client, reponse, size);
    write(1,reponse,strlen(reponse));
    write(1, "\n", 1);
    conduct_read(client, reponse, size);
    write(1,reponse,strlen(reponse) );
    write(1, "\n", 1);
    conduct_close(client);
    conduct_destroy(serveur);
    return 0;
}
