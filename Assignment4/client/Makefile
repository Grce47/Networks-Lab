compile: MyBrowser.c ../server/MyHTTP.c
	gcc MyBrowser.c -o client.out
	gcc ../server/MyHTTP.c -o ../server/server.out
run: ./client.out
	./client.out < input.txt
clean:
	rm ../**/*.out