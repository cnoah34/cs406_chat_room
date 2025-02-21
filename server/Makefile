CC=g++
CFLAGS=-Wall -Wextra
INCLUDE=-I./libraries/cpp-driver/include/ -I./libraries/nlohmann/ -I./libraries/libbcrypt/include/bcrypt/
SCYLLA=./libraries/cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./libraries/cpp-driver/build/

all: server

server: server.cpp chatDB.hpp chatUser.hpp
	$(CC) $(CFLAGS) $(INCLUDE) server.cpp \
	$(SCYLLA) \
	-lbcrypt -o server.out

clean:
	rm -f server.out
