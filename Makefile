# Makefile for generating librpc.a and binder

all: binder librpc.a

binder: binder.cc
	g++ binder.cc -o binder -pthread

librpc.a: rpc.o
	ar rcs librpc.a rpc.o

rpc.o: rpc.h rpc.cc
	g++ rpc.cc -o rpc.o -pthread

PHONY: all clean

clean:
	rm -f *.o *.a binder
