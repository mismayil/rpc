# Makefile for generating librpc.a and binder
CC = gcc
CXX = g++
AR = ar
CXXFLAGS = -Wall -g -pthread -std=c++11
ARFLAGS = rcs

OBJECTS1 = protocol.o util.o sock.o binder.o
EXEC1 = binder

OBJECTS2 = protocol.o util.o sock.o rpc.o
EXEC2 = librpc.a

OBJECTS3 = server.o server_functions.o server_function_skels.o
EXEC3 = server

OBJECTS4 = client.o
EXEC4 = client

OBJECTS = ${OBJECTS1} ${OBJECTS2} ${OBJECTS3} ${OBJECTS4}
DEPENDS = ${OBJECTS:.o=.d}
EXECS = ${EXEC1} ${EXEC2} ${EXEC3} ${EXEC4}

.PHONY: all clean ${EXECS}

all: ${EXECS}

client.o: client.c
	${CC} -c client.c -o client.o

server_functions.o: server_functions.c
	${CC} -c server_functions.c -o server_functions.o

server_function_skels.o: server_function_skels.c
	${CC} -c server_function_skels.c -o server_function_skels.o

server.o: server.c
	${CC} -c server.c -o server.o

${EXEC1}: ${OBJECTS1}
	${CXX} ${CXXFLAGS} $^ -o $@

${EXEC2}: ${OBJECTS2}
	${AR} ${ARFLAGS} $@ $^

${EXEC3}: ${OBJECTS3}
	${CXX} ${CXXFLAGS} -L. $^ -lrpc -o $@

${EXEC4}: ${OBJECTS4}
	${CXX} ${CXXFLAGS} -L. $^ -lrpc -o $@

-include ${DEPENDS}

clean:
	rm -f *.o *.d ${EXECS}
