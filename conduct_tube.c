#include "conduct_tube.h"

char* concatenation(const char* name, char* where){
    char* s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MEMOIRE(s, where);
    int error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, where);
    return s;
}

struct conduct *conduct_create(const char *name, size_t a, size_t c){
    int error;
    struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ERROR_MEMOIRE(cond, "conduct_tube.c : conduct_create : mmap conduct");
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){//si anonyme
        error = pipe(cond->tubeDescAnonyme);
        ERROR(error, "conduct_tube.c : conduct_create : pipe");
    }else { //si nommé
        char* s = concatenation(name, "conduct_tube.c : conduct_create : concatenation");
        error = mkfifo(s, 0666);
        ERROR(error, "conduct_tube.c : conduct_create : mkfifo");
        int fd = open(s, O_RDWR);
        ERROR(fd, "conduct_tube.c : conduct_create : open");
        cond->tubeNomme = fd;
        free(s);
    }
    cond->name = name;
    return cond;
}

struct conduct *conduct_open(const char *name){
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){
        perror("anonymous conduct can't be opened.\n");
        exit(EXIT_FAILURE);
    }else { //si nomme
        char* s = concatenation(name, "conduct_tube.c : conduct_create : concatenation");
        int fd = open(s, O_RDWR);
        ERROR(fd, "conduct_tube.c : conduct_open : open");
        struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED| MAP_ANONYMOUS, -1 , 0);
        cond->tubeNomme = fd;
        cond->name = name;
        free(s);
        return cond;
    }
}

void conduct_close(struct conduct *conduct){
    if(conduct->name == NULL){
        close(conduct->tubeDescAnonyme[0]);
        close(conduct->tubeDescAnonyme[1]);
    }
    else{
        close(conduct->tubeNomme);
    }
    int error = munmap(conduct, sizeof(struct cond*));
    ERROR(error, "conduct_tube.c : conduct_close : munmap struct");
}

void conduct_destroy(struct conduct *conduct){
    const char * name = conduct->name;
    conduct_close(conduct);
    if(name !=NULL){
        char *s = concatenation(name, "conduct.c : conduct_destroy: malloc name file");
        int error = unlink(s);
        ERROR(error, "conduct_tube.c : conduct_destroy : unlink");
        free(s);//pas de valeur de retour
    }
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
    if(c->name==NULL){
        count = read(c->tubeDescAnonyme[0],buf, count);
    }
    else{
        count = read(c->tubeNomme, buf, count);
    }
    return count;
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
    if(c->name ==NULL){
        count = write(c->tubeDescAnonyme[1], buf, count);
    }
    else{
        count = write(c->tubeNomme, buf, count);
    }
    return count;
}

int conduct_write_eof(struct conduct *c){
    close(c->tubeDescAnonyme[1]);
    return 0;
}
