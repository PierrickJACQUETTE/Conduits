CC = gcc
CFLAGS = -Wall
EXEC = conduct conduct_julia test_fork test_thread

all: $(EXEC)

conduct: conduct.o main.o
	$(CC) -o conduct conduct.o main.o -pthread

conduct_julia: conduct.o
	$(CC) -o conduct_julia -g -O3 -ffast-math -Wall -pthread `pkg-config --cflags gtk+-3.0` julia.c conduct.c `pkg-config --libs gtk+-3.0` -lm

test_fork: conduct.o test_fork.o
	$(CC) -o test_fork conduct.o test_fork.o -pthread

test_thread: conduct.o test_thread.o
	$(CC) -o test_thread conduct.o test_thread.o -pthread

bob: conduct.o bob.o
	$(CC) -o bob conduct.o bob.o -pthread

conduct.o: conduct.c
	$(CC) -o conduct.o -c conduct.c -Wall

main.o: main.c conduct.h
	$(CC) -o main.o -c main.c -Wall

test_fork.o: test_fork.c conduct.h
	$(CC) -o test_fork.o -c test_fork.c -Wall

test_thread.o: test_thread.c conduct.h
	$(CC) -o test_thread.o -c test_thread.c -Wall

bob.o: bob.c conduct.h
	$(CC) -o bob.o -c bob.c -Wall

clean:
	rm -rf *.o *.h.gch

mrproper:clean
	rm -f $(EXEC)
