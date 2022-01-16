#Makefile
all: echo-client-server

echo-client-server: 
				gcc echo-client.c -o echo-client -lpthread
				gcc echo-server.c -o echo-server -lpthread

clean:
		rm -f echo-client
		rm -f echo-server