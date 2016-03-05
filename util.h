#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>
#include "protocol.h"

/*
* Utility functions and definitions
*/

// error printing function
void error(std::string msg);

// copies data to the buffer
void copy(char *buf, void *v, int len=1, int type=ARG_CHAR);

// connects to the host with given address and port
int connectTo(char *address, int port);

// sends a tcp segment
int sendSegment(int sock_fd, SEGMENT *segment);

// receives a tcp segment
SEGMENT* recvSegment(int sock_fd);

class FUNC_SIGNATURE {
public:
    char *name;
    std::vector<int> iargTypes; // input arg types
    std::vector<int> oargTypes; // output arg types
    int argc;

    FUNC_SIGNATURE(char *name, int *argTypes);

    bool operator<(const FUNC_SIGNATURE &fs);
    bool operator==(const FUNC_SIGNATURE &fs);
};

/*
* casting functions
*/
short ctos(char *str);
int ctoi(char *str);
long ctol(char *str);
float ctof(char *str);
double ctod(char *str);
uint32_t htou(char *hostname);
#endif
