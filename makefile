CC = gcc
CFLAGS = -Wall
EXEC = conduct_julia test_fork test_thread test_nomme test_fork_nomme test_thread_nomme client serveur test_lots socket_julia tube_julia

all: $(EXEC)

client: client.o conduct.o
	$(CC) -o client conduct.o client.o -pthread

conduct: conduct.o
	$(CC) -o conduct conduct.o -pthread

conduct_julia: conduct.o
	$(CC) -o conduct_julia -g -O3 -ffast-math -Wall -pthread `pkg-config --cflags gtk+-3.0` julia.c conduct.c `pkg-config --libs gtk+-3.0` -lm

socket_julia:
	$(CC) -o socket_julia -g -O3 -ffast-math -Wall -pthread `pkg-config --cflags gtk+-3.0` julia.c conduct_socket.c `pkg-config --libs gtk+-3.0` -lm

tube_julia:
		$(CC) -o tube_julia -g -O3 -ffast-math -Wall -pthread `pkg-config --cflags gtk+-3.0` julia.c conduct_tube.c `pkg-config --libs gtk+-3.0` -lm

serveur: serveur.o conduct.o
	$(CC) -o serveur conduct.o serveur.o -pthread

test_fork_nomme: test_fork_nomme.o conduct.o
	$(CC) -o test_fork_nomme test_fork_nomme.o conduct.o -pthread

test_fork: conduct.o test_fork.o
	$(CC) -o test_fork conduct.o test_fork.o -pthread

test_thread_nomme: conduct.o test_thread_nomme.o
	$(CC) -o test_thread_nomme conduct.o test_thread_nomme.o -pthread

test_thread: conduct.o test_thread.o
	$(CC) -o test_thread conduct.o test_thread.o -pthread

test_nomme: conduct.o test_nomme.o
	$(CC) -o test_nomme conduct.o test_nomme.o -pthread

test_lots: conduct.o test_lots.o
	$(CC) -o test_lots conduct.o test_lots.o -pthread

client.o: client.c conduct.h
	$(CC) -o client.o -c client.c -Wall

conduct.o: conduct.c
	$(CC) -o conduct.o -c conduct.c -Wall

serveur.o: serveur.c conduct.h
	$(CC) -o serveur.o -c serveur.c -Wall

test_fork_nomme.o: test_fork_nomme.c conduct.h
	$(CC) -o test_fork_nomme.o -c test_fork_nomme.c -Wall

test_fork.o: test_fork.c conduct.h
	$(CC) -o test_fork.o -c test_fork.c -Wall

test_thread_nomme.o: test_thread_nomme.c conduct.h
	$(CC) -o test_thread_nomme.o -c test_thread_nomme.c -Wall

test_thread.o: test_thread.c conduct.h
	$(CC) -o test_thread.o -c test_thread.c -Wall

test_nomme.o: test_nomme.c conduct.h
	$(CC) -o test_nomme.o -c test_nomme.c -Wall

test.o: test_lots.c conduct.h
	$(CC) -o test_lots.o -c test_lots.c -Wall

clean:
	rm -rf *.o *.h.gch

mrproper:clean
	rm -f $(EXEC)
