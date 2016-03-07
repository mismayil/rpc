#ifndef __BINDER_H__
#define __BINDER_H__

#include "sock.h"
#include "util.h"
#include <unistd.h>

class BINDER_SOCK: public SOCK {
public:
    BINDER_SOCK(int portnum);
    int handle_request(int i);
};

struct LOCATION {
    char *hostname;
    uint32_t ipaddr;
    int port;

    LOCATION(char *hostname, int port): hostname(hostname), port(port) {
        ipaddr = htou(hostname);
    }

    bool operator==(const LOCATION &l) const {
        return (ipaddr == l.ipaddr) && (port == l.port);
    }
};

#endif
