#include <iostream>
#include <cstdlib>
#include "protocol.h"

using namespace std;

SEGMENT::SEGMENT(int length, int type, MESSAGE *message) : length(length), type(type), message(message) {}

char* SEGMENT::getBuf() {return NULL;}

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *server, int port, char *name, int *argTypes) : server(server), port(port), name(name), argTypes(argTypes) {}

char* REQ_REG_MESSAGE::getBuf() {return NULL;}

RES_REG_SUCCESS_MESSAGE::RES_REG_SUCCESS_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* RES_REG_SUCCESS_MESSAGE::getBuf() {return NULL;}

REQ_LOC_MESSAGE::REQ_LOC_MESSAGE(char *name, int *argTypes) : name(name), argTypes(argTypes) {}

char* REQ_LOC_MESSAGE::getBuf() {return NULL;}

RES_LOC_SUCCESS_MESSAGE::RES_LOC_SUCCESS_MESSAGE(char *server, int port) : server(server), port(port) {}

char* RES_LOC_SUCCESS_MESSAGE::getBuf() {return NULL;}

REQ_EXEC_MESSAGE::REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

char* REQ_EXEC_MESSAGE::getBuf() {return NULL;}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

char* RES_EXEC_SUCCESS_MESSAGE::getBuf() {return NULL;}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* REQ_TERM_MESSAGE::getBuf() {return NULL;}

char* RES_FAILURE_MESSAGE::getBuf() {return NULL;}

void error(string msg) {
    cerr << msg << endl;
    exit(1);
}
