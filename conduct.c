#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "conduct.h"

#define ERROR_STD(a,str) if ( (a < 0) ) {perror(str); exit(1);}
#define ERROR_MMAP(a) if ( (a == MAP_FAILED) ) {perror("mmap failed"); exit(1);}

struct conduct {
    const void* name;
    int c; //capacité
    int a; //atomicité
};

//Je n'ai pas fait les memset & les malloc, tu es plus à l'aise que moi à cela
struct conduct *conduct_create(const char *name, size_t a, size_t c){

    //si anonyme
    if(name == NULL){
        //ou alors PROT_NONE pour la rendre inaccessible
        void* src = mmap(NULL, 0, PROT_READ|PROT_WRITE, MAP_ANONYMOUS, -1, 0);
        ERROR_MMAP(src);

        //Faut (surement) faire un malloc ici, mais je m'y perds dsl
        conduct cond;
        cond.name = name;
        cond.c = c;
        cond.a = a;

        return &cond;

    } else if ((name != NULL) && (name[0] == '\0')) { //si name est vide
        perror("conduct name is empty\n"); // considérer comme anonyme ?
        exit(1);
    } else { //si nommé

        int fd, rc;
        struct stat st;
        void* src;

        const char* tmp = "/tmp/";
        char * s = (char*)malloc(snprintf(NULL, 0, "%s%s", tmp, name) + 1);
        sprintf(s, "%s%s", tmp, name);

        fd = open(s, O_CREAT|O_WRONLY, 0666);
        ERROR_STD(fd, "open ");

        fstat(fd, &st);
        src = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0L);
        ERROR_MMAP(src);

        conduct cond;
        cond.name = name;
        cond.c = c;
        cond.a = a;

        rc = write(fd, &cond, sizeof(&cond));
        ERROR_STD(rc, "write ");

        close(fd);

        return &cond;
    }
}

struct conduct *conduct_open(const char *name){
    if(name == NULL){
        perror("anonymous conduct can't be opened.\n");
        exit(1);
    } else if ((name != NULL) && (name[0] == '\0')) { //si name est vide
        perror("conduct name is empty\n"); // considérer comme anonyme ?
        exit(1);
    } else { //si nommé

        int fd, rc;
        struct stat st;

        const char* tmp = "/tmp/";
        char * s = (char*)malloc(snprintf(NULL, 0, "%s%s", tmp, name) + 1);
        sprintf(s, "%s%s", tmp, name);

        fd = open(s, O_CREAT|O_RDONLY, 0666);
        ERROR_STD(fd, "open ");

        fstat(fd, &st);
        conduct cond;
        //Err : cannot convert ‘conduct’ to ‘void*’ for argument ‘2’ to ‘ssize_t read(int, void*, size_t)’
        rc = read(fd, cond, st.st_size -1);

        return &cond;
    }
}

void conduct_close(struct conduct *cond){
    free(cond);
}

void conduct_destroy(struct conduct*cond){
    if(cond->name == NULL){
        conduct_close(cond);
    } else {
        char* name = (char*)cond->name;
        const char* tmp = "/tmp/";
        char * s = (char*)malloc(snprintf(NULL, 0, "%s%s", tmp, name) + 1);
        sprintf(s, "%s%s", tmp, name);
        conduct_close(cond);
        unlink(s);

        //J'ai pas de munmap car j'ai besoin du retour mmap et de la taille st.st_size
    }
}
