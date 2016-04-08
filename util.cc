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

FUNC_SIGNATURE::FUNC_SIGNATURE(char *name, int *argTypes): argTypes(argTypes) {
    strcpy(this->name, name);

    argc = 1;
    if (argTypes != NULL) {
        while (argTypes[argc-1] != ARG_TERMINATOR) {
            argc++;
        }
    }
}

bool FUNC_SIGNATURE::operator<(const FUNC_SIGNATURE &fs) const {
    int n = strcmp(name, fs.name);
    if (n < 0) return true;
    if (n > 0) return false;
    if (argc < fs.argc) return true;
    if (argc > fs.argc) return false;

    int eq = 0;
    for (int i = 0; i < argc - 1; i++) {
        int type1 = (argTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int type2 = (fs.argTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int arglen1 = argTypes[i] & ARG_LEN_MASK;
        int arglen2 = fs.argTypes[i] & ARG_LEN_MASK;

        if (type1 > type2) return false;

        if (type1 == type2) {
            if ((arglen1 == 0 && arglen2 == 0) || (arglen1 > 0 && arglen2 > 0)) eq++;
            else if (arglen1 > arglen2) return false;
        }
    }

    if (eq == (argc - 1)) return false;
    return true;
}

bool FUNC_SIGNATURE::operator==(const FUNC_SIGNATURE &fs) const {
    int n = strcmp(name, fs.name);
    if (n) return false;
    if (argc != fs.argc) return false;

    for (int i = 0; i < argc - 1; i++) {
        int type1 = (argTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int type2 = (fs.argTypes[i] & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        int arglen1 = argTypes[i] & ARG_LEN_MASK;
        int arglen2 = fs.argTypes[i] & ARG_LEN_MASK;

        if (type1 != type2) return false;
        else if ((arglen1 == 0 && arglen2 > 0) || (arglen1 > 0 && arglen2 == 0)) return false;
    }

    return true;
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
    segment->encapsulate();
    return send(sock_fd, segment->getbuf(), segment->getlen(), 0);
}

int recvSegment(int sock_fd, SEGMENT **segment) {
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
    memset(msg, 0, msglen);
    if (recv(sock_fd, msg, msglen, 0) <= 0) return RETURN_FAILURE;

    *segment = SEGMENT::decapsulate(type, msg);

    return RETURN_SUCCESS;
}

short ctos(char *str) {
    return *((short *) str);
}

int ctoi(char *str) {
    return *((int *) str);
}

long ctol(char *str) {
    return *((long *) str);
}

float ctof(char *str) {
    return *((float *) str);
}

double ctod(char *str) {
    return *((double *) str);
}

uint32_t htou(char *hostname) {
    if (hostname == NULL) return 0;
    struct hostent *host = gethostbyname(hostname);
    struct in_addr **addr_list = (struct in_addr **) host->h_addr_list;
    struct in_addr host_in_addr;
    inet_aton(inet_ntoa(*addr_list[0]), &host_in_addr);
    return host_in_addr.s_addr;
}
