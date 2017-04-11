CC = gcc
CFLAGS = -Wall
EXEC = conduct conduct_julia

all: $(EXEC)

conduct: conduct.o main.o
	$(CC) -o conduct conduct.o main.o

conduct.o: conduct.c
	$(CC) -o conduct.o -c conduct.c -Wall -pthread

main.o: main.c conduct.h
	$(CC) -o main.o -c main.c -Wall -pthread

conduct_julia: conduct.o
	gcc -o conduct_julia -g -O3 -ffast-math -Wall -pthread `pkg-config --cflags gtk+-3.0` julia.c conduct.c `pkg-config --libs gtk+-3.0` -lm

clean:
	rm -rf *.o

mrproper:clean
	rm -f $(EXEC)
