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
        if(errno==EEXIST){
            free(s);
            return conduct_open(name);
        }
        ERROR(error, "conduct_tube.c : conduct_create : mkfifo");
        error = unlink(FIFO2);
        if(errno != ENOENT){
            ERROR(error, "conduct_tube.c : conduct_create : unlink");
        }
        error = mkfifo(FIFO2, 0666);
        ERROR(error, "conduct_tube.c : conduct_create : mkfifo FIFO2");
        int fd = open(s, O_RDONLY);
        ERROR(fd, "conduct_tube.c : conduct_create : open in read");
        cond->tubeDescAnonyme[0] = fd;
        fd = open(FIFO2, O_WRONLY);
        ERROR(fd, "conduct_tube.c : conduct_create : open in write");
        cond->tubeDescAnonyme[1] = fd;
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
        int fd = open(s, O_WRONLY);
        if(errno==ENOENT){
            sleep(1);
            fd = open(s, O_WRONLY);
        }
        ERROR(fd, "conduct_tube.c : conduct_open : open in write");
        int fd2 = open(FIFO2, O_RDONLY|O_NONBLOCK);
        ERROR(fd2, "conduct_tube.c : conduct_open : open in read");
        struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED| MAP_ANONYMOUS, -1 , 0);
        ERROR_MEMOIRE(cond, "conduct_tube.c : conduct_open : mmap");
        cond->tubeDescAnonyme[0] = fd2;
        cond->tubeDescAnonyme[1] = fd;
        cond->name = name;
        free(s);
        return cond;
    }
}

void conduct_close(struct conduct *conduct){
    int error  = close(conduct->tubeDescAnonyme[0]);
    ERROR(error, "conduct.c : conduct_close : close 0 ");
    error = close(conduct->tubeDescAnonyme[1]);
    ERROR(error, "conduct.c : conduct_close : close 1 ");
    error = munmap(conduct, sizeof(struct cond*));
    ERROR(error, "conduct_tube.c : conduct_close : munmap struct");
}

void conduct_destroy(struct conduct *conduct){
    const char * name = conduct->name;
    conduct_close(conduct);
    if(name !=NULL){
        char *s = concatenation(name, "conduct.c : conduct_destroy: malloc name file");
        int error = unlink(s);
        ERROR(error, "conduct_tube.c : conduct_destroy : unlink");
        error = unlink(FIFO2);
        ERROR(error, "conduct_tube.c : conduct_destroy : unlink FIFO2");
        free(s);//pas de valeur de retour
    }
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
    return read(c->tubeDescAnonyme[0],buf, count);
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
    return write(c->tubeDescAnonyme[1], buf, count);
}

int conduct_write_eof(struct conduct *c){
    int error = close(c->tubeDescAnonyme[1]);
    ERROR(error, "conduct.c : conduct_close : close 1 ");
    return 0;
}
