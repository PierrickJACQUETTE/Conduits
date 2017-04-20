#include <sys/wait.h>

#include "conduct.h"

int main(int argc, char const *argv[]) {

    pid_t pid;
    struct conduct* essai1 = conduct_create(NULL, 5, sizeof(char)*12);
    pid = fork();
    if(pid < 0){
        perror("fork failed");
        exit(1);
    } else if(pid == 0){
        char* buff = malloc(sizeof(char)*4);
        buff = "test";
        conduct_write(essai1, buff, sizeof(buff));
        conduct_close(essai1);
        exit(0);
    } else {
        wait(NULL);
        char* buff2 = malloc(sizeof(char)*4);
        buff2 = "azrt";
        char* reponse = malloc(sizeof(char)*4);
        conduct_write(essai1, buff2, sizeof(buff2));
        conduct_read(essai1, reponse, sizeof(reponse));
        write(1,reponse,strlen(reponse));
        write(1, "\n", 1);
        conduct_read(essai1, reponse, sizeof(reponse));
        write(1,reponse,strlen(reponse) );
        write(1, "\n", 1);
        free(reponse);
        conduct_destroy(essai1);
    }
    return 0;
}
