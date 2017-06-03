#ifndef __CONDUCT_H__
#define __CONDUCT_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/uio.h>

#define ERROR(a,str) if (a < 0 && errno != 0) {perror(str); exit(EXIT_FAILURE);}
#define ERROR_FREE(a,str,buff, cond) if (a < 0 && errno != 0) {free(buff); conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_MAP(a,str,cond) if (a < 0 && errno != 0) {conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_FD(a,str,fd) if (a < 0 && errno != 0) {close(fd); perror(str); exit(EXIT_FAILURE);}
#define ERROR_FD_MAP(a,str,fd,cond) if (a < 0 && errno != 0) {close(fd); conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_ARGUMENT_I(a,str) if(a<1){errno = EINVAL; perror(str); exit(EXIT_FAILURE);}
#define ERROR_ARGUMENT_S(a,str) if(a == NULL){errno = EINVAL; perror(str); exit(EXIT_FAILURE);}
#define ERROR_MEMOIRE(a,str) if(a == NULL && errno != 0){perror(str); exit(EXIT_FAILURE);}
#define ERROR_MEMOIRE_FREE(a,str,buff,cond) if(a == NULL && errno != 0){free(buff); conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_MEMOIRE_FD(a,str,fd) if(a == NULL && errno != 0){close(fd); perror(str); exit(EXIT_FAILURE);}
#define ERROR_MEMOIRE_MAP(a,str,cond) if(a == NULL && errno != 0){conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_THREAD(a,str,cond) if(a !=0){conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define ERROR_THREAD_MAP(a,str,cond) if(a !=0){conduct_close(cond); perror(str); exit(EXIT_FAILURE);}
#define TMP "/tmp/"

struct conduct {
    const void* name;
    size_t capacite; //capacité
    size_t a; //atomicité si a<=c
    size_t contenu;
    size_t teteDeLecture;
    bool eof;
    pthread_mutex_t verrou;
    pthread_mutexattr_t attrVerrou;
    pthread_cond_t aEcrit;
    pthread_condattr_t attrEcrit;
    pthread_cond_t aLu;
    pthread_condattr_t attrLu;
};

struct conduct *conduct_create(const char *name, size_t a, size_t c);
struct conduct *conduct_open(const char *name);
ssize_t conduct_read(struct conduct *c, void *buf, size_t count);
ssize_t conduct_readv(struct conduct *c, const struct iovec *iov, int iovcnt);
ssize_t conduct_write(struct conduct *c, const void *buf, size_t count);
ssize_t conduct_writev(struct conduct *c, const struct iovec *iov, int iovcnt);
int conduct_write_eof(struct conduct *c);
void conduct_close(struct conduct *conduct);
void conduct_destroy(struct conduct *conduct);

#endif
