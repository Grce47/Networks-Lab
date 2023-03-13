server.out: server.c libmsocket.a
	gcc server.c -o server.out -L. -lmsocket
libmsocket.a: mysocket.o
	ar rcs libmsocket.a mysocket.o
mysocket.o: mysocket.c
	gcc -c mysocket.c -o mysocket.o
clean:
	rm *.out *.a *.o

