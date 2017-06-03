#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main( int argc, char **argv ){
    int fd;
    if((fd = open("resultat.txt", O_CREAT|O_APPEND|O_WRONLY,0666)) < 0){
        perror( "open" );
        exit( EXIT_FAILURE );
    }
    char* buff = malloc(sizeof(char)*6);
    memset(buff, 0, 7);
    int j;
    for(j=1; j<2; j++){
        if(j==0){
            system("gcc -Wall conduct.c client.c -o test -pthread");
        }
        else if(j==1){
            system("gcc -Wall conduct_tube.c client.c -o test -pthread");
        }
        else if(j==2){
            system("gcc -Wall conduct_socket.c client.c -o test -pthread");
        }
        struct timespec start, stop;
        memset(&start, 0, sizeof(start));
        memset(&stop, 0, sizeof(stop));
        if( clock_gettime( CLOCK_REALTIME, &start) < 0 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        int i;
        for(i=0; i<500; i++){
            system("./test coucou les gens");
        }
        if( clock_gettime( CLOCK_REALTIME, &stop) < 0 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        sprintf(buff,"%.4lf", (stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec )/1E9);
        write(fd,buff, 4);
        write(fd, "\n", 1);
    }
    free(buff);
    close(fd);
    return( EXIT_SUCCESS );
}
