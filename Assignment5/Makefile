test: client.out server.out
client.out: client.c libmsocket.a
	gcc client.c -o client.out -L. -lmsocket -lpthread 
server.out: server.c libmsocket.a
	gcc server.c -o server.out -L. -lmsocket -lpthread
libmsocket.a: mysocket.o
	ar rcs libmsocket.a mysocket.o
mysocket.o: mysocket.c
	gcc -c mysocket.c -o mysocket.o
clean:
	rm *.out *.a *.o 