#include "conduct.h"

void* first(void* ptr){
    struct conduct* n = conduct_create("test", 5, sizeof(char)*12);
    char* buff = malloc(sizeof(char)*4);
    buff = "test";
    conduct_write(n, buff, sizeof(buff));
    return NULL;
}

void* second(void* ptr){
    struct conduct* n = conduct_open("test");
    char* buff2 = malloc(sizeof(char)*4);
    buff2 = "azrt";
    char* reponse = malloc(sizeof(char)*4);
    conduct_write(n, buff2, sizeof(buff2));
    conduct_read(n, reponse, sizeof(reponse));
    write(1,reponse,strlen(reponse));
    write(1, "\n", 1);
    conduct_read(n, reponse, sizeof(reponse));
    write(1,reponse,strlen(reponse) );
    write(1, "\n", 1);
    free(reponse);
    conduct_destroy(n);
    return NULL;
}

int main(int argc, char const *argv[]) {
    pthread_t th;
    int error, i;
    pthread_t tab[2];

    error = pthread_create(&th, NULL, first, NULL);
    if(error){
        perror("pthread_create");
    }
    tab[0] = th;
    error = pthread_create(&th, NULL, second, NULL);
    if(error){
        perror("pthread_create");
    }
    tab[1] = th;
    for(i=0; i<2;i++){
        pthread_join(tab[i], NULL);
    }
    return 0;
}