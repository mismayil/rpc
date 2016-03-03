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
    char *msgbuf = message->marshall();
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

SEGMENT* SEGMENT::decapsulate(int type, char *msg) {
    MESSAGE *message;

    switch (type) {
        case REQUEST_REGISTER:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_LOCATION:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_EXECUTE:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_TERMINATE:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REGISTER_SUCCESS:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REGISTER_FAILURE:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case LOCATION_SUCCESS:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case LOCATION_FAILURE:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case EXECUTE_SUCCESS:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case EXECUTE_FAILURE:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
    }

    SEGMENT *segment = new SEGMENT(type, message);

    return segment;
}

MESSAGE::~MESSAGE() {
    if (buf) delete [] buf;
}

MESSAGE* MESSAGE::unmarshall(char *msg) { return NULL;}

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes) : serverID(serverID), port(port), name(name), argTypes(argTypes) {}

char* REQ_REG_MESSAGE::marshall() {
    int argc = 0;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = strlen(serverID) + 1 + SIZE_INT + strlen(name) + 1 + argc * SIZE_INT + 1;
    buf = new char[len];
    char *bufptr = buf;

    copy(bufptr, (void *) serverID, strlen(serverID));
    bufptr += strlen(serverID);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    copy(bufptr, (void *) &port, 1, ARG_INT);
    bufptr += SIZE_INT;

    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    copy(bufptr, (void *) argTypes, argc, ARG_INT);

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* REQ_REG_MESSAGE::unmarshall(char *msg) {
    // char *msgptr = msg;
    // int len = 0;
    // char intbuf[SIZE_INT];
    // char *serverID;
    // int port;
    // // get server identifier
    // while (*(msgptr + len) != NULL_TERMINATOR) {
    //     len++;
    // }
    //
    // serverID = new char[len];
    // copy(serverID, (void *) msgptr, len);
    // serverID[len-1] = NULL_TERMINATOR;
    // msgptr += len;
    //
    // // get server port
    // copy(intbuf, (void *) msgptr, SIZE_INT);
    // port = atoi(intbuf);
    // msgptr += SIZE_INT;
    //
    // // get arg types
    return NULL;

}

RES_REG_SUCCESS_MESSAGE::RES_REG_SUCCESS_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* RES_REG_SUCCESS_MESSAGE::marshall() {
    return buf;
}

MESSAGE* RES_REG_SUCCESS_MESSAGE::unmarshall(char *msg) {
    return NULL;
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

MESSAGE* REQ_LOC_MESSAGE::unmarshall(char *msg) {
    return NULL;
}

RES_LOC_SUCCESS_MESSAGE::RES_LOC_SUCCESS_MESSAGE(char *serverID, int port) : serverID(serverID), port(port) {}

char* RES_LOC_SUCCESS_MESSAGE::marshall() {
    return buf;
}

MESSAGE* RES_LOC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    return NULL;
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
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                len += arglen * SIZE_CHAR;
                break;
            case ARG_SHORT:
                len += arglen * SIZE_SHORT;
                break;
            case ARG_INT:
                len += arglen * SIZE_INT;
                break;
            case ARG_LONG:
                len += arglen * SIZE_LONG;
                break;
            case ARG_FLOAT:
                len += arglen * SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                len += arglen * SIZE_DOUBLE;
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
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                copy(bufptr, (void *) args[i], arglen, ARG_CHAR);
                bufptr += arglen * SIZE_CHAR;
                break;
            case ARG_SHORT:
                copy(bufptr, (void *) args[i], arglen, ARG_SHORT);
                bufptr += arglen * SIZE_SHORT;
                break;
            case ARG_INT:
                copy(bufptr, (void *) args[i], arglen, ARG_INT);
                bufptr += arglen * SIZE_INT;
                break;
            case ARG_LONG:
                copy(bufptr, (void *) args[i], arglen, ARG_LONG);
                bufptr += arglen * SIZE_LONG;
                break;
            case ARG_FLOAT:
                copy(bufptr, (void *) args[i], arglen, ARG_FLOAT);
                bufptr += arglen * SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                copy(bufptr, (void *) args[i], arglen, ARG_DOUBLE);
                bufptr += arglen * SIZE_DOUBLE;
                break;
        }
    }

    buf[len-1] = NULL_TERMINATOR;
    return buf;
}

MESSAGE* REQ_EXEC_MESSAGE::unmarshall(char *msg) {
    return NULL;
}

char* REQ_TERM_MESSAGE::marshall() {
    buf = new char[1];
    buf[0] = NULL_TERMINATOR;
    return buf;
}

MESSAGE* REQ_TERM_MESSAGE::unmarshall(char *msg) {
    return NULL;
}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

char* RES_EXEC_SUCCESS_MESSAGE::marshall() {
    return buf;
}

MESSAGE* RES_EXEC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    return NULL;
}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* RES_FAILURE_MESSAGE::marshall() {
    return buf;
}

MESSAGE* RES_FAILURE_MESSAGE::unmarshall(char *msg) {
    return NULL;
}

int connectTo(char *address, int port) {
    int sock_fd;
    struct sockaddr_in sock_addr;
    struct hostent *host;

    // open a socket for connection
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return ESOCKET;

    // clear the address
    memset(&sock_addr, 0, sizeof(sock_addr));

    // get the address info
    host = gethostbyname(address);
    if (!host) return ENOHOST;

    // populate the address
    sock_addr.sin_family = AF_INET;
    memcpy(&sock_addr.sin_addr.s_addr, host->h_addr, host->h_length);
    sock_addr.sin_port = htons(port);

    // connect to the host
    if (connect(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) return ECONNECT;

    return sock_fd;
}

int sendSegment(int sock_fd, SEGMENT *segment) {
    char *buf = segment->encapsulate();
    return send(sock_fd, buf, strlen(buf) + 1, 0);
}

SEGMENT* recvSegment(int sock_fd) {
    int length, type;
    char intbuf[SIZE_INT];

    // receive the length
    recv(sock_fd, intbuf, SIZE_INT, 0);
    length = atoi(intbuf);

    // receive the type
    recv(sock_fd, intbuf, SIZE_INT, 0);
    type = atoi(intbuf);

    // receive the message
    int msglen = length - SIZE_INT * 2;
    char msg[msglen];
    recv(sock_fd, msg, msglen, 0);

    SEGMENT *segment = SEGMENT::decapsulate(type, msg);

    return segment;
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
