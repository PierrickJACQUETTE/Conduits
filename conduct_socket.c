#include "conduct_socket.h"

char* concatenation(const char* name, char* where){
    char* s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MEMOIRE(s, where);
    int error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, where);
    return s;
}

struct conduct *conduct_create(const char *name, size_t a, size_t c){
    struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ERROR_MEMOIRE(cond, "conduct_socket.c : conduct_create : mmap conduct");
    cond->name = name;
    int error;
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){
        error = socketpair(AF_UNIX , SOCK_DGRAM, 0, cond->socket);
        ERROR(error, "conduct_socket.c : conduct_create : socketpair");
        cond->serveur = false;
    }
    else{
        error = socket(AF_UNIX, SOCK_STREAM, 0);
        ERROR(error, "conduct_socket.c : conduct_create : socket");
        cond->socket[0] = error;
        struct sockaddr_un sun;
        sun.sun_family = AF_UNIX;
        char* s = concatenation(name, "conduct_socket.c : conduct_create : concatenation");
        char* error2 = strcpy(sun.sun_path, s);
        ERROR_MEMOIRE(error2, "conduct_socket.c : conduct_create : strcpy");
        free(s);
        int len = sizeof(sun.sun_family) + strlen(sun.sun_path);
        error = bind(cond->socket[0], (struct sockaddr *)&sun, len);
        ERROR(error, "conduct_socket.c : conduct_create : bind");
        error = listen(cond->socket[0], 1);
        ERROR(error, "conduct_socket.c : conduct_create : listen");
        int sunlen = sizeof(sun);
        error = accept(cond->socket[0], (struct sockaddr *)&sun, (socklen_t *)&sunlen);
        ERROR(error, "conduct_socket.c : conduct_create : accept");
        cond->socket[1] = error;
        cond->serveur = true;
    }
    return cond;
}

struct conduct *conduct_open(const char *name){
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){
        perror("anonymous conduct can't be opened.\n");
        exit(EXIT_FAILURE);
    }else { //si nomme
        char* s = concatenation(name, "conduct_socket.c : conduct_open : concatenation");
        struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED| MAP_ANONYMOUS, -1 , 0);
        ERROR_MEMOIRE(cond, "conduct_socket.c : conduct_open : mmap");
        int error = socket(AF_UNIX, SOCK_STREAM, 0);
        ERROR(error, "conduct_socket.c : conduct_open : socket");
        cond->socket[1] = error;
        cond->name = name;
        struct sockaddr_un sun;
        sun.sun_family = AF_UNIX;
        char* error2 = strcpy(sun.sun_path, s);
        ERROR_MEMOIRE(error2, "conduct_socket.c : conduct_open : strcpy");
        int len = sizeof(sun.sun_family) + strlen(sun.sun_path);
        error = connect(cond->socket[1], (struct sockaddr *)&sun, len);
        if(errno==ENOENT){
            sleep(1);
            error = connect(cond->socket[1], (struct sockaddr *)&sun, len);
        }
        ERROR(error, "conduct_socket.c : conduct_open : connect");
        cond->serveur = false;
        free(s);
        return cond;
    }
}

void conduct_close(struct conduct *conduct){
    int error  = close(conduct->socket[1]);
    ERROR(error, "conduct_socket.c : conduct_close : close 1")
    error = munmap(conduct, sizeof(struct conduct));
    ERROR(error, "conduct_tube.c : conduct_close : munmap struct");
}

void conduct_destroy(struct conduct *conduct){
    const char * name =NULL;
    if(conduct->name != NULL){
        name = conduct->name;
    }
    int error  = close(conduct->socket[0]);
    ERROR(error, "conduct_socket.c : conduct_destroy : close 0");
    conduct_close(conduct);
    if(name != NULL){
        char* s = concatenation(name, "conduct.c : conduct_destroy: malloc name file");
        error = unlink(s);
        ERROR(error, "conduct_socket.c : conduct_destroy : unlink");
        free(s);//pas de valeur de retour
    }
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
    ssize_t lu = 0;
    while(lu <= 0){
        if(c->name == NULL){
            lu = recv(c->socket[0], buf, count,0);
        }
        else{
            lu = recv(c->socket[1], buf, count,0);
        }
		ERROR(lu, "conduct_socket.c : conduct_read : recv ");
    }
    return lu;
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
    ssize_t ecrit = 0;
    ecrit = send(c->socket[1], buf, count,0);
   	ERROR(ecrit, "conduct_socket.c : conduct_write : send ");
    return ecrit;
}

int conduct_write_eof(struct conduct *c){
    int error = close(c->socket[1]);
    ERROR(error, "conduct_socket.c : conduct_write_eof : close 1 ");
    return 0;
}
