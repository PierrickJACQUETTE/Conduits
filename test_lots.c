#include "conduct.h"

int main(int argc, char const *argv[]) {

    struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
    if(serveur==NULL){
        printf("NULL\n");
    }
    int size = 10;

    char *str0 = malloc(sizeof(char)*size);
    memset(str0, 0, size+1);
    str0 = "azertyuioz";

    char *str1 = malloc(sizeof(char)*size);
    memset(str1, 0, size+1);
    str1 = "qsdfghjklm";
    struct iovec iov[2];

    iov[0].iov_base = str0;
    iov[0].iov_len = size;
    iov[1].iov_base = str1;
    iov[1].iov_len = size;

    conduct_writev(serveur, iov, 2);

    struct conduct* client = conduct_open("serveur");

    struct iovec iov2[2];

    char* reponse = malloc(sizeof(char)*size);
    memset(reponse, 0, size+1);

    char* reponse2 = malloc(sizeof(char)*size);
    memset(reponse2, 0, size+1);

    iov2[0].iov_base = reponse;
    iov2[0].iov_len = size;
    iov2[1].iov_base = reponse2;
    iov2[1].iov_len = size;

    conduct_readv(client, iov2, 2);

    write(1,reponse,strlen(reponse));
    write(1, "\n", 1);

    write(1,reponse2,strlen(reponse2));
    write(1, "\n", 1);

    conduct_close(client);
    conduct_destroy(serveur);
    return 0;

}
