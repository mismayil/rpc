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

SEGMENT::SEGMENT(int type, MESSAGE *message) : type(type), message(message) {
    char *msgbuf = message->getbuf();
    length = INT_SIZE * 2 + buflen(msgbuf);
    buf = new char[length];
    copy_int_to_buf(length, buf);
    copy_int_to_buf(type, buf + INT_SIZE);
    memcpy(buf + INT_SIZE * 2, msgbuf, buflen(msgbuf));
}

SEGMENT::~SEGMENT() {
    if (buf) delete [] buf;
}

char* SEGMENT::getbuf() { return buf; }

MESSAGE::~MESSAGE() {
    if (buf) delete [] buf;
}

char* MESSAGE::getbuf() { return buf; }

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes) : serverID(serverID), port(port), name(name), argTypes(argTypes) {}

RES_REG_SUCCESS_MESSAGE::RES_REG_SUCCESS_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

REQ_LOC_MESSAGE::REQ_LOC_MESSAGE(char *name, int *argTypes) : name(name), argTypes(argTypes) {
    int len = 0;

    if (argTypes != NULL) {
        while (argTypes[len] != 0) {
            len++;
        }
    }

    buf = new char[MAX_NAME_LENGTH + 1 + len * INT_SIZE];
    memcpy(buf, name, buflen(name));

    for (int i = 0; i < len; i++) {
        copy_int_to_buf(argTypes[i], buf + MAX_NAME_LENGTH + 1 + i * INT_SIZE);
    }
}

RES_LOC_SUCCESS_MESSAGE::RES_LOC_SUCCESS_MESSAGE(char *serverID, int port) : serverID(serverID), port(port) {
    buf = new char[MAX_NAME_LENGTH + 1 + INT_SIZE];
    memcpy(buf, serverID, buflen(serverID));
    copy_int_to_buf(port, buf + buflen(serverID));
}

REQ_EXEC_MESSAGE::REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {
    int len = 0;

    if (argTypes != NULL) {
        while (argTypes[len] != 0) {
            len++;
        }
    }

    // to be continued
    buf = new char[MAX_NAME_LENGTH + 1 + len * INT_SIZE];
}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

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
    char *buf = segment->getbuf();
    return send(sock_fd, buf, buflen(buf), 0);
}

SEGMENT* recvSegment(int sock_fd) {
    return NULL;
}

void error(char *msg) {
    cerr << msg << endl;
    exit(1);
}

int buflen(char *buf) {
    return strlen(buf) + 1;
}

void copy_int_to_buf(int n, char *buf) {
    stringstream ss;
    ss << n;
    memcpy(buf, ss.str().c_str(), sizeof(int));
}
