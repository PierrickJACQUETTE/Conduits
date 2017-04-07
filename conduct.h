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

#define ERROR(a,str) if (a < 0 && errno != 0) {perror(str); exit(0);}
#define ERROR_MMAP(a) if (a == MAP_FAILED && errno != 0) {perror("mmap failed"); exit(0);}
#define ERROR_MALLOC(a, str) if(a == NULL && errno != 0){perror(str); exit(0);}
#define TMP "/tmp/"

struct conduct {
  const void* name;
  size_t c; //capacité
  size_t a; //atomicité si a<=c
  void* retourMmap; //pour destroy
  int fd; //file descriptor
  size_t sizeFstat; //sizeFstat
};

struct conduct *conduct_create(const char *name, size_t a, size_t c);
struct conduct *conduct_open(const char *name);
ssize_t conduct_read(struct conduct *c, void *buf, size_t count);
ssize_t conduct_write(struct conduct *c, const void *buf, size_t count);
int conduct_write_eof(struct conduct *c);
void conduct_close(struct conduct *conduct);
void conduct_destroy(struct conduct *conduct);

#endif
