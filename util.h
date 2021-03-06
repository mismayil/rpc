#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>
#include "protocol.h"

/*
* Utility functions and definitions
*/

// debug functions
#define INFO(msg) do { cout << "[INFO] (" << __FILE__ << ":" << __LINE__ << ") " << msg << endl; } while(0)
#define DEBUG(msg, val)  do { cout << "[DEBUG] (" << __FILE__ << ":" << __LINE__ << ") "<< msg << "=" << val << endl; } while(0)

// connects to the host with given address and port
int connectTo(char *address, int port);

// sends a tcp segment
int sendSegment(int sock_fd, SEGMENT *segment);

// receives a tcp segment
int recvSegment(int sock_fd, SEGMENT **segment);

/*
* casting functions
*/
short ctos(char *str);
int ctoi(char *str);
long ctol(char *str);
float ctof(char *str);
double ctod(char *str);

// hostname to uint32_t ip address
uint32_t htou(char *hostname);

// defines a function signature
struct FUNC_SIGNATURE {
    char name[MAX_FUNC_NAME_LEN];
    int *argTypes;
    int argc;
    FUNC_SIGNATURE(char *name, int *argTypes);
    bool operator<(const FUNC_SIGNATURE &fs) const;
    bool operator==(const FUNC_SIGNATURE &fs) const;
};

// defines a server location
struct LOCATION {
    char hostname[MAX_SERVER_NAME_LEN];
    uint32_t ipaddr;
    int port;
    LOCATION(): ipaddr(0), port(0) {}
    LOCATION(char *hostname, int port): port(port) {
        strcpy(this->hostname, hostname);
        ipaddr = htou(hostname);
    }

    bool operator==(const LOCATION &l) const {
        return (ipaddr == l.ipaddr) && (port == l.port);
    }
};

#endif
