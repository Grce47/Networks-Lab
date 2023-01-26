client: client.c	
	gcc client.c -o client
	./client 8000
lb: lb.c
	gcc lb.c -o lb
	./lb 8000 8001 8002
server1: server
	./server 8001
server2: server
	./server 8002
server: server.c
	gcc server.c -o server
clean:
	rm client server lb