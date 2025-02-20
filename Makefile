CC=g++
CFLAGS=-Wall -Wextra
INCLUDE=-I./cpp-driver/include/ -I./nlohmann/

all: server client

server: server.cpp chatDB.h
	$(CC) $(CFLAGS) $(INCLUDE) server.cpp \
	./cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./cpp-driver/build/ \
	-o server.out

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client.out

clean:
	rm -f server.out client.out
