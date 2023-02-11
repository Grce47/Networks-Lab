compile: MyHTTP.c ../client/MyBrowser.c
	gcc MyHTTP.c -o server.out
	gcc ../client/MyBrowser.c -o ../client/client.out
run: server.out
	./server.out 8080
clean:
	rm ../**/*.out