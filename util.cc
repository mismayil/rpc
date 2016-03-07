#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "util.h"

using namespace std;

FUNC_SIGNATURE::FUNC_SIGNATURE(char *name, int *argTypes): name(name) {
    argc = 0;
    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            int io = argTypes[argc] & ARG_IO_MASK;
            if (io == (1 << ARG_INPUT)) iargTypes.push_back(argTypes[argc]);
            if (io == (1 << ARG_OUTPUT)) oargTypes.push_back(argTypes[argc]);
            argc++;
        }
    }
}

bool FUNC_SIGNATURE::operator<(const FUNC_SIGNATURE &fs) const {
    int n = strcmp(name, fs.name);
    if (n < 0) return true;
    if (n > 0) return false;
    if (iargTypes.size() < fs.iargTypes.size()) return true;
    if (iargTypes.size() > fs.iargTypes.size()) return false;

    unsigned int eq = 0;
    for (unsigned int i = 0; i < iargTypes.size(); i++) {
        int type1 = (iargTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int type2 = (fs.iargTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int arglen1 = iargTypes[i] & ARG_LEN_MASK;
        int arglen2 = fs.iargTypes[i] & ARG_LEN_MASK;

        if (type1 > type2) return false;
        if (type1 == type2) {
            if ((arglen1 == 0 && arglen2 == 0) || (arglen1 > 0 && arglen2 > 0)) eq++;
            else if (arglen1 > arglen2) return false;
        }
    }

    if (eq == iargTypes.size()) return false;
    return true;
}

bool FUNC_SIGNATURE::operator==(const FUNC_SIGNATURE &fs) const {
    int n = strcmp(name, fs.name);
    if (n) return false;
    if (iargTypes.size() != fs.iargTypes.size()) return false;

    for (unsigned int i = 0; i < iargTypes.size(); i++) {
        int type1 = (iargTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int type2 = (fs.iargTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int arglen1 = iargTypes[i] & ARG_LEN_MASK;
        int arglen2 = fs.iargTypes[i] & ARG_LEN_MASK;

        if (type1 != type2) return false;
        else if ((arglen1 == 0 && arglen2 > 0) || (arglen1 > 0 && arglen2 == 0)) return false;
    }

    return true;
}

int connectTo(char *address, int port) {
    INFO("in connectTo");
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
    segment->encapsulate();
    return send(sock_fd, segment->getbuf(), segment->getlen(), 0);
}

int recvSegment(int sock_fd, SEGMENT *segment) {
    int length, type;
    char intbuf[SIZE_INT];

    // receive the length
    if (recv(sock_fd, intbuf, SIZE_INT, 0) <= 0) return RETURN_FAILURE;
    length = ctoi(intbuf);

    // receive the type
    if (recv(sock_fd, intbuf, SIZE_INT, 0) <= 0) return RETURN_FAILURE;
    type = ctoi(intbuf);

    // receive the message
    int msglen = length - SIZE_INT;
    char *msg = new char[msglen];
    if (recv(sock_fd, msg, msglen, 0) <= 0) return RETURN_FAILURE;

    segment = SEGMENT::decapsulate(type, msg);

    return RETURN_SUCCESS;
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

short ctos(char *str) {
    stringstream ss;
    ss << str;
    short s;
    ss >> s;
    return s;
}

int ctoi(char *str) {
    stringstream ss;
    ss << str;
    int i;
    ss >> i;
    return i;
}

long ctol(char *str) {
    stringstream ss;
    ss << str;
    long l;
    ss >> l;
    return l;
}

float ctof(char *str) {
    stringstream ss;
    ss << str;
    float f;
    ss >> f;
    return f;
}

double ctod(char *str) {
    stringstream ss;
    ss << str;
    double d;
    ss >> d;
    return d;
}

uint32_t htou(char *hostname) {
    struct hostent *host = gethostbyname(hostname);
    struct in_addr **addr_list = (struct in_addr **) host->h_addr_list;
    struct in_addr host_in_addr;
    inet_aton(inet_ntoa(*addr_list[0]), &host_in_addr);
    return host_in_addr.s_addr;
}
