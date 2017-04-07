#include "conduct.h"

struct conduct *conduct_create(const char *name, size_t a, size_t c){
  void* src;
  struct conduct* cond = (struct conduct*) malloc(sizeof(struct conduct));
  //si anonyme
  if(name == NULL){
    //ou alors PROT_NONE pour la rendre inaccessible
    src = mmap(NULL, 0, PROT_READ|PROT_WRITE, MAP_ANONYMOUS, -1, 0);
    ERROR_MMAP(src);

    ERROR_MALLOC(cond, "conduct.c : conduct_create : malloc conduct");
    cond->name = name;
    cond->c = c;
    cond->a = a;
    cond->fd = -1;
    cond->retourMmap = src;
    cond->sizeFstat = 0;

    return cond;
  } else if ((name != NULL) && (name[0] == '\0')) { //si name est vide
    perror("conduct name is empty\n"); // considérer comme anonyme ?
    exit(1);
  } else { //si nommé

    int fd, error;
    struct stat st;
    char* s;

    s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MALLOC(s, "conduct.c : conduct_create : malloc name file");
    error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, "conduct.c : conduct_create : sprintf");
    fd = open(s, O_CREAT|O_WRONLY|O_EXCL, 0666);
    if(errno==EEXIST){
      cond = conduct_open(name);
    }
    else{
      ERROR(fd, "conduct.c : conduct_create : open");
      error = fstat(fd, &st);
      ERROR(error, "conduct.c : conduct_create : fstat");
      src = mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, fd, 0L);
      ERROR_MMAP(src);

      ERROR_MALLOC(s, "conduct.c : conduct_create : malloc conduct");
      cond->name = name;
      cond->c = c;
      cond->a = a;
      cond->retourMmap = src;
      cond->fd = fd;
      cond->sizeFstat = st.st_size;

      error = write(fd, &cond, sizeof(&cond));
      ERROR(error, "conduct.c : conduct_create : write");
    }
    return cond;
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

    struct stat st;
    char *s;
    int error, fd;

    s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MALLOC(s, "conduct.c : conduct_open : malloc name file");
    error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, "conduct.c : conduct_open : sprintf");

    fd = open(s, O_RDONLY, 0666);
    ERROR(fd, "conduct.c : conduct_open : open");

    error = fstat(fd, &st);
    ERROR(error, "conduct.c : conduct_open : fstat");

    struct conduct* cond =NULL;
    error = read(fd, cond, st.st_size -1);
    ERROR(error, "conduct.c : conduct_open : read");
    return cond;
  }
}

void conduct_close(struct conduct *cond){
  int error;
  error = munmap(cond->retourMmap, cond->sizeFstat);
  ERROR(error, "conduct.c : conduct_close : munmap");
  error = close(cond->fd);
  ERROR(error, "conduct.c : conduct_close : close");
  free(cond);
}

void conduct_destroy(struct conduct*cond){
  if(cond->name == NULL){
    conduct_close(cond);
  } else {
    char* name, *s;
    int error;

    name = (char*)cond->name;
    s = malloc(sizeof(TMP)+(unsigned)strlen(name)*sizeof(char));
    ERROR_MALLOC(s, "conduct.c : conduct_destroy: malloc name file");
    error = sprintf(s, "%s%s", TMP, name);
    ERROR(error, "conduct.c : conduct_destroy : sprintf");
    conduct_close(cond);
    error = unlink(s);
    ERROR(error, "conduct.c : conduct_destroy : unlink");
  }
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){return 0;}
ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){return 0;}
int conduct_write_eof(struct conduct *c){return 0;}
