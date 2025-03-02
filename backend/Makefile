# Compiler flags
CC=g++
CFLAGS=-Wall -Wextra 

# Include directories
INCLUDE=-I./include/cpp-driver/include/ \
	-I./include/nlohmann/ \
	-I./include/libbcrypt/include/bcrypt/ \
	-I./include/cpp-httplib/ \
	-I./include/jwt-cpp \
	-I./database \
	-I./api

# Preprocessor flags for jwt-cpp (disables base64 & picojson)
CFLAGS += -DJWT_DISABLE_BASE64 -DJWT_DISABLE_PICOJSON

# ScyllaDB linking
SCYLLA_LIB=./include/cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./include/cpp-driver/build/

# bcrypt linking
LIBS=-lbcrypt

# Header files
#HEADERS=./database/chatDB.hpp $(wildcard ./api/*.hpp)

all: server

server: ./src/server.o #$(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) ./src/server.o $(SCYLLA_LIB) $(LIBS) -o server.out

./src/server.o: ./src/server.cpp #$(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) -c ./src/server.cpp -o ./src/server.o

clean:
	rm -f server.out ./src/*.o
