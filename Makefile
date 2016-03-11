# Makefile for generating librpc.a and binder
CC = gcc
CXX = g++
AR = ar
CXXFLAGS = -Wall -g -MMD -pthread -std=c++0x
ARFLAGS = rcs

OBJECTS1 = protocol.o util.o scheduler.o sock.o binder.o
EXEC1 = binder

OBJECTS2 = protocol.o util.o scheduler.o sock.o rpc.o
EXEC2 = librpc.a

# OBJECTS3 = server.o server_functions.o server_function_skels.o
# EXEC3 = server
#
# OBJECTS4 = client.o
# EXEC4 = client

OBJECTS = ${OBJECTS1} ${OBJECTS2}
DEPENDS = ${OBJECTS:.o=.d}
EXECS = ${EXEC1} ${EXEC2}

.PHONY: all clean

all: ${EXECS}

${EXEC1}: ${OBJECTS1}
	${CXX} ${CXXFLAGS} $^ -o $@

${EXEC2}: ${OBJECTS2}
	${AR} ${ARFLAGS} $@ $^

# ${EXEC3}: ${OBJECTS3}
# 	${CXX} -L. $^ -lrpc -pthread -o $@
#
# ${EXEC4}: ${OBJECTS4}
# 	${CXX} -L. $^ -lrpc -pthread -o $@

-include ${DEPENDS}

clean:
	rm -f *.o *.d ${EXECS}
