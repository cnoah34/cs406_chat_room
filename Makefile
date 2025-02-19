CC=g++
CFLAGS= -Wall -Wextra

all: server client

server: server.cpp chatDB.h
	$(CC) $(CFLAGS) server.cpp \
	./cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./cpp-driver/build/ -I ./cpp-driver/include/ \
	-o server.out

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client.out

clean:
	rm -f server.out client.out
