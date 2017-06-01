#include <sys/wait.h>

#include "conduct.h"

int main(int argc, char const *argv[]) {

    pid_t pid;
    struct conduct* essai1 = conduct_create(NULL, 20, sizeof(char)*64);
    int size = 10;
    pid = fork();
    if(pid < 0){
        perror("fork failed");
        exit(1);
    } else if(pid == 0){
        char* buff = malloc(sizeof(char)*size);
        memset(buff, 0, size+1);
        buff = "azertyuioz";
        conduct_write(essai1, buff, size);
        conduct_close(essai1);
        exit(0);
    } else {
        wait(NULL);
        char* buff2 = malloc(sizeof(char)*size);
        memset(buff2, 0, size+1);
        buff2 = "qsdfghjklm";
        char* reponse = malloc(sizeof(char)*size);
        memset(reponse, 0, size+1);
        conduct_write(essai1, buff2, size);
        conduct_read(essai1, reponse, size);
        write(1,reponse,strlen(reponse));
        write(1, "\n", 1);
        conduct_read(essai1, reponse, size);
        write(1,reponse,strlen(reponse) );
        write(1, "\n", 1);
        free(reponse);
        conduct_destroy(essai1);
    }
    return 0;
}
