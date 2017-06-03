#include "conduct.h"

int main(int argc, char const *argv[]) {

    struct conduct* serveur = conduct_create("serveur", 20, sizeof(char)*64);
    if(serveur==NULL){
        printf("NULL\n");
    }
    int size = 10, i;

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

    for (i = 0; i < 2; i++){
        write(1, (char *) iov2[i].iov_base, iov2[i].iov_len);
        write(1, "\n", 1);
    }

    conduct_close(client);
    conduct_destroy(serveur);
    return 0;

}
