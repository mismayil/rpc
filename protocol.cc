#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "protocol.h"

using namespace std;

SEGMENT::SEGMENT(int type, MESSAGE *message) : type(type), message(message) {}

SEGMENT::~SEGMENT() {
    if (buf) delete [] buf;
}

char* SEGMENT::encapsulate() {
    REQ_MESSAGE *req_message = dynamic_cast<REQ_MESSAGE*>(message);
    char *msgbuf = req_message->marshall();
    length = SIZE_INT * 2 + strlen(msgbuf) + 1;
    buf = new char[length];
    char *bufptr = buf;
    copy(bufptr, (void *) &length, 1, ARG_INT);
    bufptr += SIZE_INT;
    copy(bufptr, (void *) &type, 1, ARG_INT);
    bufptr += SIZE_INT;
    copy(bufptr, (void *) msgbuf, strlen(msgbuf));
    buf[length-1] = NULL_TERMINATOR;
    return buf;
}

void SEGMENT::decapsulate() {}

MESSAGE::~MESSAGE() {
    if (buf) delete [] buf;
}

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes) : serverID(serverID), port(port), name(name), argTypes(argTypes) {}

char* REQ_REG_MESSAGE::marshall() {
    return buf;
}

RES_REG_SUCCESS_MESSAGE::RES_REG_SUCCESS_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

void RES_REG_SUCCESS_MESSAGE::unmarshall() {

}

REQ_LOC_MESSAGE::REQ_LOC_MESSAGE(char *name, int *argTypes) : name(name), argTypes(argTypes) {}

char* REQ_LOC_MESSAGE::marshall() {
    int argc = 0;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = strlen(name) + 1 + argc * SIZE_INT + 1;
    buf = new char[len];
    char *bufptr = buf;
    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;
    copy(bufptr, (void *) argTypes, argc, ARG_INT);
    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

RES_LOC_SUCCESS_MESSAGE::RES_LOC_SUCCESS_MESSAGE(char *serverID, int port) : serverID(serverID), port(port) {}

void RES_LOC_SUCCESS_MESSAGE::unmarshall() {

}

REQ_EXEC_MESSAGE::REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

char* REQ_EXEC_MESSAGE::marshall() {
    int argc = 0;
    int argType, type, arglen;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = strlen(name) + 1 + argc * SIZE_INT + 1;

    for (int i = 0; i < argc - 1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;

        switch (type) {
            case ARG_CHAR:
                len += arglen > 0 ? arglen * SIZE_CHAR : SIZE_CHAR;
                break;
            case ARG_SHORT:
                len += arglen > 0 ? arglen * SIZE_SHORT : SIZE_SHORT;
                break;
            case ARG_INT:
                len += arglen > 0 ? arglen * SIZE_INT : SIZE_INT;
                break;
            case ARG_LONG:
                len += arglen > 0 ? arglen * SIZE_LONG : SIZE_LONG;
                break;
            case ARG_FLOAT:
                len += arglen > 0 ? arglen * SIZE_FLOAT : SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                len += arglen > 0 ? arglen * SIZE_DOUBLE : SIZE_DOUBLE;
                break;
        }
    }

    buf = new char[len];
    char *bufptr = buf;

    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;
    copy(bufptr, (void *) argTypes, argc, ARG_INT);
    bufptr += argc * SIZE_INT;

    for (int i = 0; i < argc - 1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;

        switch (type) {
            case ARG_CHAR:
                copy(bufptr, (void *) args[i], )
                break;
            case ARG_SHORT:
                len += arglen > 0 ? arglen * SIZE_SHORT : SIZE_SHORT;
                break;
            case ARG_INT:
                len += arglen > 0 ? arglen * SIZE_INT : SIZE_INT;
                break;
            case ARG_LONG:
                len += arglen > 0 ? arglen * SIZE_LONG : SIZE_LONG;
                break;
            case ARG_FLOAT:
                len += arglen > 0 ? arglen * SIZE_FLOAT : SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                len += arglen > 0 ? arglen * SIZE_DOUBLE : SIZE_DOUBLE;
                break;
        }
    }

    buf[len-1] = NULL_TERMINATOR;
    return buf;
}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

void RES_EXEC_SUCCESS_MESSAGE::unmarshall() {

}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

void RES_FAILURE_MESSAGE::unmarshall() {

}

int connectTo(char *address, int port) {
    int sock_fd;
    struct sockaddr_in sock_addr;
    struct hostent *host;

    // open a socket for connection
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return ESOCKET;

    // clear binder address
    memset(&sock_addr, 0, sizeof(sock_addr));

    // get binder address info
    host = gethostbyname(address);
    if (!host) return ENOHOST;

    // populate binder address
    sock_addr.sin_family = AF_INET;
    memcpy(&sock_addr.sin_addr.s_addr, host->h_addr, host->h_length);
    sock_addr.sin_port = htons(port);

    // connect to the binder
    if (connect(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) return ECONNECT;

    return sock_fd;
}

int sendSegment(int sock_fd, SEGMENT *segment) {
    char *buf = segment->encapsulate();
    return send(sock_fd, buf, buflen(buf), 0);
}

SEGMENT* recvSegment(int sock_fd) {
    return NULL;
}

void error(char *msg) {
    cerr << msg << endl;
    exit(1);
}

void copy(char *buf, void *v, int len, int type) {
    stringstream ss;
    int size = 0;
    char *c; short *s; int *i; long *l;
    float *f; double *d;

    for (int k = 0; k < len; k++) {
        switch (type) {
            case ARG_CHAR:
                c = (char *) v;
                ss << *(c + k);
                size += SIZE_CHAR;
                break;
            case ARG_SHORT:
                s = (short *) v;
                ss << *(s + k);
                size += SIZE_SHORT;
                break;
            case ARG_INT:
                i = (int *) v;
                ss << *(i + k);
                size += SIZE_INT;
                break;
            case ARG_LONG:
                l = (long *) v;
                ss << *(l + k);
                size += SIZE_LONG;
                break;
            case ARG_FLOAT:
                f = (float *) v;
                ss << *(f + k);
                size += SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                d = (double *) v;
                ss << *(d + k);
                size += SIZE_DOUBLE;
                break;
        }
    }

    memcpy(buf, ss.str().c_str(), size);
}
