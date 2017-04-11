#include "conduct.h"

struct conduct *conduct_create(const char *name, size_t a, size_t c){
  void* src;
  struct conduct* cond = (struct conduct*) malloc(sizeof(struct conduct));
  ERROR_MALLOC(cond, "conduct.c : conduct_create : malloc conduct");
  //si anonyme
  if(name == NULL){
    //ou alors PROT_NONE pour la rendre inaccessible
    src = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ERROR_MMAP(src);
    cond->name = name;
    cond->capacite = c;
    cond->a = a;
    cond->fd = -1;
    cond->retourMmap = src;
    cond->sizeFstat = 1024;
    cond->teteDeLecture = 0;
    cond->contenu = 0;
    cond->eof = false;
    pthread_mutex_init(&cond->verrou, NULL);
    pthread_cond_init(&cond->aEcrit, NULL);
    pthread_cond_init(&cond->aLu, NULL);

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
      src = mmap(NULL, st.st_size,PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0L);
      ERROR_MMAP(src);

      ERROR_MALLOC(s, "conduct.c : conduct_create : malloc conduct");
      cond->name = name;
      cond->capacite = c;
      cond->a = a;
      printf("%p\n",src );
      cond->retourMmap = src;
      cond->fd = fd;
      cond->sizeFstat = st.st_size;
      cond->contenu = 0;
      cond->teteDeLecture = 0;
      cond->eof = false;
      pthread_mutex_init(&cond->verrou, NULL);
      pthread_cond_init(&cond->aEcrit, NULL);
      pthread_cond_init(&cond->aLu, NULL);

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
  if(cond->fd !=-1){
    error = close(cond->fd);
    ERROR(error, "conduct.c : conduct_close : close");
  }
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

ssize_t myRead(struct conduct *c, void *buf, size_t count){
  int error;
  size_t fin, debut;
  count = (count < c->contenu)? count : c->contenu;
  fin = c->capacite - c->teteDeLecture;
  fin = (count < fin )? count : fin;
  debut = count - fin;
  //memmove si zone de memoire se chevauchent par exemple evite une erreur
  memcpy(buf, c->retourMmap+(c->teteDeLecture%c->capacite), fin);
  //erreur a tester
  //memmove si zone de memoire se chevauchent par exemple evite une erreur
  memcpy(c->retourMmap, buf+fin, debut);
  //erreur a tester
  c->teteDeLecture = (c->teteDeLecture+count)%c->capacite;
  c->contenu -= count;
  error = pthread_cond_signal(&c->aLu);
  ERROR_THREAD(error, "conduct.c : myRead : pthread_cond_signal");
  return count;
}

ssize_t myWrite(struct conduct *c, const void *buf, size_t count){
  int error;
  size_t fin, debut;
  count = (count+c->contenu > c->capacite)? c->capacite - c->contenu : count;
  fin = c->capacite - (c->teteDeLecture + c->contenu)%c->capacite;
  fin = (count < fin )? count : fin;
  debut = count - fin;
  //memmove si zone de memoire se chevauchent par exemple evite une erreur
  memcpy(c->retourMmap+ c->teteDeLecture + c->contenu, buf, fin);
  //erreur a tester
  //memmove si zone de memoire se chevauchent par exemple evite une erreur
  memcpy(c->retourMmap, buf+fin, debut);
  //erreur a tester
  c->contenu += count;
  error = pthread_cond_signal(&c->aEcrit);
  ERROR_THREAD(error, "conduct.c : myWrite : pthread_cond_signal");
  return count;
}

ssize_t conduct_read(struct conduct *c, void *buf, size_t count){
  int error;
  ssize_t lu = 0;
  error = pthread_mutex_lock(&c->verrou);
  ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_lock");
  if(c->contenu == 0 && c->eof == false){
    while(c->contenu == 0 || c-> eof == false){
      error = pthread_cond_wait(&c->aEcrit, &c->verrou);
      ERROR(error, "conduct.c : conduct_read : pthread_cond_wait");
    }
    if(c->eof == true){
      lu = 0;
    }
    else{
      lu = myRead(c, buf, count);
    }
  }
  else if(c->contenu == 0 && c->eof == true){
    lu = 0;
  }
  else if(c->contenu > 0){
    lu = myRead(c, buf, count);
  }
  error = pthread_mutex_unlock(&c->verrou);
  ERROR_THREAD(error, "conduct.c : conduct_read : pthread_mutex_unlock");
  return lu;
}


ssize_t conduct_write(struct conduct *c, const void *buf, size_t count){
  int error;
  ssize_t ecrit = 0;
  error = pthread_mutex_lock(&c->verrou);
  ERROR_THREAD(error, "conduct.c : conduct_write : pthread_mutex_lock");
  if(c->eof == true){
    ecrit = -1;
    errno = EPIPE;
  }
  else{
    if(count <= c->a){
      while(c->capacite - c->contenu < count){
        error = pthread_cond_wait(&c->aLu, &c->verrou);
        ERROR(error, "conduct.c : conduct_write : pthread_cond_wait");
      }
      ecrit = myWrite(c, buf, count);
    }
    else if(count > c->a){
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
  int error;
  error = pthread_mutex_lock(&c->verrou);
  ERROR_THREAD(error, "conduct.c : conduct_write_eof : pthread_mutex_lock");
  if(c->eof == false){
    c->eof = true;
  }
  pthread_mutex_unlock(&c->verrou);
  ERROR_THREAD(error, "conduct.c : conduct_write_eof : pthread_mutex_unlock");
  return 0;
}
