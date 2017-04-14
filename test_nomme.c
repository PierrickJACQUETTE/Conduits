#include "conduct.h"

int main(int argc, char const *argv[]) {
    struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
    if(serveur==NULL){
        printf("NULL\n");
    }
    char* buff = malloc(sizeof(char)*4);
    buff = "test";
    struct conduct* client = conduct_open("serveur");
    conduct_write(serveur, buff, sizeof(buff));
    buff = "azrt";
    char* reponse = malloc(sizeof(char)*4);
    conduct_write(client, buff, sizeof(buff));
    conduct_read(client, reponse, sizeof(reponse));
    write(1,reponse,strlen(reponse));
    write(1, "\n", 1);
    conduct_read(client, reponse, sizeof(reponse));
    write(1,reponse,strlen(reponse) );
    write(1, "\n", 4);
    conduct_close(client);
    conduct_destroy(serveur);
    return 0;
}
