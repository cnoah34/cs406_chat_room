# Compiler flags
CC=g++
CFLAGS=-Wall -Wextra -DWITH_OPENSSL=1

# VCPKG for jwt-cpp
VCPKG_DIR=/home/colby/.vcpkg/installed/x64-linux
VCPKG_INCLUDE=$(VCPKG_DIR)/include
VCPKG_LIB=$(VCPKG_DIR)/lib

# Include directories (isystem is the same as I, just without compiler warnings)
INCLUDE=-I./include \
	-I./database \
	-I./api \
	-isystem$(VCPKG_INCLUDE)

# ScyllaDB linking
SCYLLA_LIB=./include/cpp-driver/build/libscylla-cpp-driver.so -Wl,-rpath,./include/cpp-driver/build/ 

# Linking these
LIBS=-lbcrypt -lz -lssl -lcrypto -ldl ./include/uSockets/*.o

# For VCPKG installed libraries
LDFLAGS=-L$(VCPKG_LIB)

all: server

server: ./src/server.o
	$(CC) $(CFLAGS) $(INCLUDE) ./src/server.o $(SCYLLA_LIB) $(LIBS) $(LDFLAGS) -o server.out

./src/server.o: ./src/server.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c ./src/server.cpp -o ./src/server.o

clean:
	rm -f server.out ./src/*.o
