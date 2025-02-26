CC=g++
CFLAGS=-Wall -Wextra
INCLUDE=-I./libraries/cpp-driver/include/ -I./libraries/nlohmann/ -I./libraries/libbcrypt/include/bcrypt/
SCYLLA=./libraries/cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./libraries/cpp-driver/build/
HEADERS=chatDB.hpp chatUser.hpp chatMessage.hpp chatRoom.hpp checkFields.hpp

all: server

server: server.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) server.cpp \
	$(SCYLLA) \
	-lbcrypt -o server.out

clean:
	rm -f server.out
