#include "conduct.h"

int main(int argc, char const *argv[]) {
    struct conduct* cond = conduct_create("bob", 5, sizeof(char)*12);
    char* buff = malloc(sizeof(char)*4);
    buff = "test";
    conduct_write(cond, buff, sizeof(buff));
    char* reponse = malloc(sizeof(char)*4);
    conduct_read(cond, reponse, sizeof(reponse));
    write(1,reponse,strlen(reponse));
    write(1, "\n", 1);
    unlink("/tmp/bob");
    return 0;
}
