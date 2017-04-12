#include "conduct.h"

int main(int argc, char const *argv[]) {
  char* buff = malloc(sizeof(char)*4);
  char* buff2 = malloc(sizeof(char)*4);
  struct conduct* essai1 = conduct_create(NULL, 5, sizeof(buff)*2);
  buff = "test";
  buff2 = "azrt";
  char* reponse = malloc(sizeof(char)*4);
  char* reponse2 = malloc(sizeof(char)*4);
  conduct_write(essai1, buff, sizeof(buff));
  conduct_write(essai1, buff2, sizeof(buff2));
  conduct_read(essai1, reponse, sizeof(reponse));
  printf("%s\n",reponse );
  conduct_read(essai1, reponse2, sizeof(reponse2));
  printf("%s\n",reponse2 );
  free(reponse);
  free(reponse2);
  conduct_destroy(essai1);
  return 0;
}
