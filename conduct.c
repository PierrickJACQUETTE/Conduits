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
    printf("Read ! sizeof %ld, c: %p, ret: %p\n", sizeof(struct conduct), c, (void*)(c)+sizeof(struct conduct)+(c->teteDeLecture + c->contenu)%c->capacite);
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
    printf("sizeof %ld, c: %p, ret: %p\n", sizeof(struct conduct), c, (void*)(c)+sizeof(struct conduct)+(c->teteDeLecture + c->contenu)%c->capacite);
    void* errorMem = memcpy((void*)(c)+sizeof(struct conduct)+ (c->teteDeLecture + c->contenu)%c->capacite, buf, fin);
    ERROR_MEMOIRE(errorMem, "conduct.c : myRead : memcpy : fin");
    printf("niak\n");
    errorMem = memcpy((void*)(c)+sizeof(struct conduct), buf+fin, debut);
    ERROR_MEMOIRE(errorMem, "conduct.c : myRead : memcpy : fin");
    c->contenu += count;
    printf("capacite : %zu\n", c->capacite);
    int error = msync(c, sizeof(struct conduct)+c->capacite*sizeof(char), MS_SYNC);
    ERROR(error, "conduct.c : myWrite : msync");
    printf("icic\n");
    error = pthread_cond_signal(&c->aEcrit);
    ERROR_THREAD(error, "conduct.c : myWrite : pthread_cond_signal");
    return count;
}

struct conduct *conduct_create(const char *name, size_t a, size_t c){
    int error;
    struct conduct* cond;
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
    error = pthread_mutexattr_init(&cond->attrVerrou);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutexattr_init");
    error = pthread_mutexattr_setpshared(&cond->attrVerrou, PTHREAD_PROCESS_SHARED);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutexattr_setpshared");
    error = pthread_mutex_init(&cond->verrou, &cond->attrVerrou);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_mutex_init");
    pthread_condattr_init(&cond->attrEcrit);
    pthread_condattr_setpshared(&cond->attrEcrit, PTHREAD_PROCESS_SHARED);
    error = pthread_cond_init(&cond->aEcrit, &cond->attrEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_cond_init : aEcrit");
    pthread_condattr_init(&cond->attrLu);
    pthread_condattr_setpshared(&cond->attrLu, PTHREAD_PROCESS_SHARED);
    error = pthread_cond_init(&cond->aLu, &cond->attrLu);
    ERROR_THREAD(error, "conduct.c : conduct_create : pthread_cond_init : aLu");
    cond->retourMmap = (void*)(cond)+sizeof(struct conduct);
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
        free(s);
        ERROR(fd, "conduct.c : conduct_open : open");
        int error = fstat(fd, &st);
        ERROR(error, "conduct.c : conduct_open : fstat");
        struct conduct* cond = mmap(NULL, sizeof(struct conduct), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        error = close(fd);
        ERROR(error, "conduct.c : conduct_open : close");
        return cond;
    }
}

void conduct_close(struct conduct *cond){
    int error = munmap(cond, sizeof(struct cond*));
    ERROR(error, "conduct.c : conduct_close : munmap struct");
}

void conduct_destroy(struct conduct* cond){
    int error;
    if(cond->name != NULL){
        char *s = concatenation(cond->name, "conduct.c : conduct_destroy: malloc name file");
        error = unlink(s);
        ERROR(error, "conduct.c : conduct_destroy : unlink");
        free(s);//pas de valeur de retour
    }
    error = pthread_cond_destroy(&cond->aLu);
    ERROR_THREAD(error, "conduct.c : conduct_close : pthread_cond_destroy : aLu");
    pthread_condattr_destroy(&cond->attrLu);
    error = pthread_cond_destroy(&cond->aEcrit);
    ERROR_THREAD(error, "conduct.c : conduct_close : pthread_cond_destroy : aEcrit");
    pthread_condattr_destroy(&cond->attrLu);
    error = pthread_mutex_destroy(&cond->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_close : pthread_mutex_destroy");
    pthread_mutexattr_destroy(&cond->attrVerrou);
    conduct_close(cond);
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
    ssize_t lu = 0;
    int error = pthread_mutex_lock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_lock");
    if(c->contenu == 0 && c->eof == false){
        while(c->contenu == 0 && c->eof == false){
            error = pthread_cond_wait(&c->aEcrit, &c->verrou);
            ERROR(error, "conduct.c : conduct_read : pthread_cond_wait");
        }
        if(c->eof == true){
            lu = 0;
        }else{
            lu = myRead(c, buf, count);
        }
    }else if(c->contenu == 0 && c->eof == true){
        lu = 0;
    }else if(c->contenu > 0){
        lu = myRead(c, buf, count);
    }
    error = pthread_mutex_unlock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_unlock");
    return lu;
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
    ssize_t ecrit = 0;
    int error = pthread_mutex_lock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write : pthread_mutex_lock");
    if(c->eof == true){
        ecrit = -1;
        errno = EPIPE;
    }else{
        if(count <= c->a){
            while(c->capacite - c->contenu < count){
                error = pthread_cond_wait(&c->aLu, &c->verrou);
                ERROR(error, "conduct.c : conduct_write : pthread_cond_wait");
            }
            ecrit = myWrite(c, buf, count);
        }else if(count > c->a){
            if(c->capacite == c->contenu){
                while(c->capacite - c->contenu <= 0){
                    error = pthread_cond_wait(&c->aLu, &c->verrou);
                    ERROR_THREAD(error, "conduct.c : conduct_write : pthread_cond_wait");
                }
            }
            ecrit = myWrite(c, buf, count);
        }
    }
    error = pthread_mutex_unlock(&c->verrou);
    ERROR_THREAD(error, "conduct.c : conduct_write : pthread_mutex_unlock");
    return ecrit;
}

int conduct_write_eof(struct conduct *c){
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
