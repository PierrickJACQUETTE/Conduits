#ifndef __CONDUCT_TUBE_H__
#define __CONDUCT_TUBE_H__

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define ERROR(a,str) if (a < 0 && errno != 0) {perror(str); exit(EXIT_FAILURE);}
#define ERROR_MEMOIRE(a, str) if(a == NULL && errno != 0){perror(str); exit(EXIT_FAILURE);}
#define ERROR_THREAD(a,str) if(a !=0){perror(str); exit(EXIT_FAILURE);}
#define TMP "/tmp/"
#define FIFO2 "/tmp/fifo2"

struct conduct {
    const void* name;
    int tubeDescAnonyme[2];
};

struct conduct *conduct_create(const char *name, size_t a, size_t c);
struct conduct *conduct_open(const char *name);
ssize_t conduct_read(struct conduct *c, void *buf, size_t count);
ssize_t conduct_write(struct conduct *c, const void *buf, size_t count);
int conduct_write_eof(struct conduct *c);
void conduct_close(struct conduct *conduct);
void conduct_destroy(struct conduct *conduct);

#endif
