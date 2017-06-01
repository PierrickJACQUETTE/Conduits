#include "conduct.h"

char* concatenation(const char* name, char* where){
    char* s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MEMOIRE(s, where);
    int error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, where);
    return s;
}

ssize_t myRead(struct conduct *c, void *buf, size_t count){
    count = (count < c->contenu)? count : c->contenu;
    size_t fin = c->capacite - c->teteDeLecture;
    fin = (count < fin )? count : fin;
    size_t debut = count - fin;
    void* errorMem = memcpy(buf, (void*)(c)+sizeof(struct conduct)+(c->teteDeLecture%c->capacite), fin);
    ERROR_MEMOIRE(errorMem, "conduct.c : myRead : memcpy : fin");
    errorMem = memcpy((void*)(c)+sizeof(struct conduct), buf+fin, debut);
    ERROR_MEMOIRE(errorMem, "conduct.c : myRead : memcpy : debut");
    c->teteDeLecture = (c->teteDeLecture+count)%c->capacite;
    c->contenu -= count;
    int error = msync(c, sizeof(struct conduct)+c->capacite*sizeof(char), MS_SYNC);
    ERROR(error, "conduct.c : myRead : msync");
    error = pthread_cond_signal(&c->aLu);
    ERROR_THREAD(error, "conduct.c : myRead : pthread_cond_signal");
    return count;
}

ssize_t myWrite(struct conduct *c, const void *buf, size_t count){
    count = (count+c->contenu > c->capacite)? c->capacite - c->contenu : count;
    size_t fin = c->capacite - (c->teteDeLecture + c->contenu)%c->capacite;
    fin = (count < fin )? count : fin;
    size_t debut = count - fin;
    void* errorMem = memcpy((void*)(c)+sizeof(struct conduct)+ (c->teteDeLecture + c->contenu)%c->capacite, buf, fin);
    ERROR_MEMOIRE(errorMem, "conduct.c : myWrite : memcpy : fin");
    errorMem = memcpy((void*)(c)+sizeof(struct conduct), buf+fin, debut);
    ERROR_MEMOIRE(errorMem, "conduct.c : myWrite : memcpy : debut");
    c->contenu += count;
    int error = msync(c, sizeof(struct conduct)+c->capacite*sizeof(char), MS_SYNC);
    ERROR(error, "conduct.c : myWrite : msync");
    error = pthread_cond_signal(&c->aEcrit);
    ERROR_THREAD(error, "conduct.c : myWrite : pthread_cond_signal");
    return count;
}

struct conduct *conduct_create(const char *name, size_t a, size_t c){
    ERROR_ARGUMENT_I(a, "a doit etre >0");
    ERROR_ARGUMENT_I(c, "c doit etre >0");
    int error;
    struct conduct* cond = NULL;
    size_t length = sizeof(struct conduct)+c*sizeof(void);
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){//si anonyme
        cond = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ERROR_MEMOIRE(cond, "conduct.c : conduct_create : mmap conduct");
    }else { //si nommé
        char* s = concatenation(name, "conduct.c : conduct_create : concatenation");
        int fd = open(s, O_CREAT|O_RDWR|O_EXCL, 0666);
        free(s);
        if(errno==EEXIST){
            return conduct_open(name);
        }else{
            ERROR(fd, "conduct.c : conduct_create : open");
            error = ftruncate(fd, length);
            ERROR(error, "conduct.c : conduct_create : ftruncate");
            cond =  mmap(NULL,  length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0L);
            ERROR_MEMOIRE(cond, "conduct.c : conduct_create : mmap conduct");
            error = close(fd);
            ERROR(error, "conduct.c : conduct_create : close");
        }
    }
    cond = memset(cond, 0, length);
    ERROR_MEMOIRE(cond, "conduct.c : conduct_create : memset");
    error = pthread_mutexattr_init(&cond->attrVerrou);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutexattr_init");
    error = pthread_mutexattr_setpshared(&cond->attrVerrou, PTHREAD_PROCESS_SHARED);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutexattr_setpshared");
    error = pthread_mutex_init(&cond->verrou, &cond->attrVerrou);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutex_init");
    error = pthread_condattr_init(&cond->attrEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_condattr_init : aEcrit");
    error = pthread_condattr_setpshared(&cond->attrEcrit, PTHREAD_PROCESS_SHARED);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_condattr_setpshared : aEcrit");
    error = pthread_cond_init(&cond->aEcrit, &cond->attrEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_cond_init : aEcrit");
    error = pthread_condattr_init(&cond->attrLu);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_condattr_init : aLu");
    error = pthread_condattr_setpshared(&cond->attrLu, PTHREAD_PROCESS_SHARED);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_condattr_setpshared : aLu");
    error = pthread_cond_init(&cond->aLu, &cond->attrLu);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_cond_init : aLu");
    cond->contenu = 0;
    cond->teteDeLecture = 0;
    cond->eof = false;
    cond->name = name;
    cond->capacite = c;
    cond->a = a;
    return cond;
}

struct conduct *conduct_open(const char *name){
    if(name == NULL || ((name != NULL) && (name[0] == '\0'))){
        perror("anonymous conduct can't be opened.\n");
        exit(EXIT_FAILURE);
    }else { //si nommé
        struct stat st;
        char *s = concatenation(name, "conduct.c : conduct_open : malloc name file");
        int fd = open(s, O_RDWR, 0666);
        if(errno==ENOENT){
            sleep(1);
            fd = open(s, O_RDWR, 0666);
        }
        ERROR(fd, "conduct.c : conduct_open : open");
        free(s);
        int error = fstat(fd, &st);
        ERROR(error, "conduct.c : conduct_open : fstat");
        struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_MEMOIRE(cond, "conduct.c : conduct_open : mmap conduct");
        error = close(fd);
        ERROR(error, "conduct.c : conduct_open : close");
        return cond;
    }
}

void conduct_close(struct conduct *cond){
    ERROR_ARGUMENT_S(cond, "cond doit etre non null");
    int error = munmap(cond, sizeof(struct cond*));
    ERROR(error, "conduct.c : conduct_close : munmap struct");
}

void conduct_destroy(struct conduct* cond){
    ERROR_ARGUMENT_S(cond, "cond doit etre non null");
    int error;
    if(cond->name != NULL){
        char *s = concatenation(cond->name, "conduct.c : conduct_destroy: malloc name file");
        error = unlink(s);
        ERROR(error, "conduct.c : conduct_destroy : unlink");
        free(s);//pas de valeur de retour
    }
    error = pthread_cond_destroy(&cond->aLu);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_cond_destroy : aLu");
    error = pthread_condattr_destroy(&cond->attrLu);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_condattr_destroy : aLu");
    error = pthread_cond_destroy(&cond->aEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_cond_destroy : aEcrit");
    error = pthread_condattr_destroy(&cond->attrEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_condattr_destroy : aEcrit");
    error = pthread_mutex_destroy(&cond->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_mutex_destroy");
    pthread_mutexattr_destroy(&cond->attrVerrou);
    ERROR_THREAD(error, "conduct.c : conduct_destroy : pthread_mutexattr_destroy");
    conduct_close(cond);
}
ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
    ERROR_ARGUMENT_S(c, "c doit etre non null");
    ERROR_ARGUMENT_S(buf, "buf doit etre non null");
    ERROR_ARGUMENT_I(count, "count doit etre >0");
    ssize_t lu = 0;
    int error = pthread_mutex_lock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_lock");
    if(c->contenu == 0 && c->eof == false){
        while(c->contenu == 0 && c->eof == false){
            error = pthread_cond_wait(&c->aEcrit, &c->verrou);
            ERROR_THREAD(error, "conduct.c : conduct_read : pthread_cond_wait");
        }
    }
    if(c->eof == false || c->contenu >= 0){
        lu = myRead(c, buf, count);
    }
    error = pthread_mutex_unlock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_unlock");
    return lu;
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
    ERROR_ARGUMENT_S(c, "c doit etre non null");
    ERROR_ARGUMENT_S(buf, "buf doit etre non null");
    ERROR_ARGUMENT_I(count, "count doit etre >0");
    ssize_t ecrit = 0;
    int error = pthread_mutex_lock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write : pthread_mutex_lock");
    if(count <= c->a){
        while(c->capacite - c->contenu < count && c->eof == false){
            error = pthread_cond_wait(&c->aLu, &c->verrou);
            ERROR_THREAD(error, "conduct.c : conduct_write : pthread_cond_wait");
        }
    }else if(count > c->a && c->capacite == c->contenu){
        while(c->capacite - c->contenu <= 0 && c->eof == false){
            error = pthread_cond_wait(&c->aLu, &c->verrou);
            ERROR_THREAD(error, "conduct.c : conduct_write : pthread_cond_wait");
        }
    }
    if(c->eof == true){
        ecrit = -1;
        errno = EPIPE;
    }else{
        ecrit = myWrite(c, buf, count);
    }
    error = pthread_mutex_unlock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write : pthread_mutex_unlock");
    return ecrit;
}

int conduct_write_eof(struct conduct *c){
    ERROR_ARGUMENT_S(c, "c doit etre non null");
    int error = pthread_mutex_lock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write_eof : pthread_mutex_lock");
    if(c->eof == false){
        c->eof = true;
        error = pthread_cond_signal(&c->aEcrit);
        ERROR_THREAD(error, "conduct.c : conduct_write_eof : pthread_cond_signal");
    }
    error = pthread_mutex_unlock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write_eof : pthread_mutex_unlock");
    return 0;
}

ssize_t conduct_readv(struct conduct *c, const struct iovec *iov, int iovcnt){
    int res = 0, i;
    for(i = 0; i < iovcnt; i++){
        res += conduct_read(c, iov[i].iov_base, iov[i].iov_len);
    }
    return res;
}

ssize_t conduct_writev(struct conduct *c, const struct iovec *iov, int iovcnt){

    int res = 0, i, error;
    for(i = 0; i < iovcnt; i++){
        res += iov[i].iov_len;
    }

    char* s = malloc(res*sizeof(char));
    ERROR_MEMOIRE(s, "conduct.c : conduct_writev : malloc");

    for(i = 0; i < iovcnt; i++){
        error = sprintf(s, "%s%s", s, (char*)iov[i].iov_base);
        ERROR(error, "conduct.c : conduct_writev : sprintf");
    }

    res = conduct_write(c, s, strlen(s));
    return res;
}
