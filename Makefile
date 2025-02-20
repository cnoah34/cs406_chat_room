CC=g++
CFLAGS=-Wall -Wextra
INCLUDE=-I./cpp-driver/include/ -I./nlohmann/ -I./libbcrypt/include/bcrypt/

all: server client

server: server.cpp chatDB.hpp
	$(CC) $(CFLAGS) $(INCLUDE) server.cpp \
	./cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./cpp-driver/build/ \
	-lbcrypt -o server.out

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client.out

clean:
	rm -f server.out client.out
