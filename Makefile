# Makefile for generating librpc.a and binder
CC = gcc
CXX = g++
AR = ar
CXXFLAGS = -L. -Wall -g -pthread
ARFLAGS = rcs

OBJS1 = protocol.o binder.o
EXEC1 = binder

OBJS2 = protocol.o rpc.o
EXEC2 = librpc.a

OBJS3 = server.o server_functions.o server_functions_skels.o
EXEC3 = server

OBJS4 = client.o
EXEC4 = client

OBJS = ${OBJS1} ${OBJS2} ${OBJS3} ${OBJS4}
DEPENDS = ${OBJS:.o=.d}
EXECS = ${EXEC1} ${EXEC2}

PHONY: all clean

all: ${EXECS}

client.o: client.c
	${CC} -c client.c -o client.o

server_functions.o: server_functions.c
	${CC} -c server_functions.c -o server_functions.o

server_functions_skels.o: server_functions_skels.c
	${CC} -c server_functions_skels.c -o server_functions_skels.o

server.o: server.c
	${CC} -c server.c -o server.o

${EXEC1}: ${OBJS1}
	${CXX} ${CXXFLAGS} $^ -o $@

${EXEC2}: ${OBJS2}
	${AR} ${ARFLAGS} $@ $^

${EXEC3}: ${OBJS3}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

${EXEC4}: ${OBJS4}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

-include ${DEPENDS}

clean:
	rm -f *.o *.d ${EXECS}
