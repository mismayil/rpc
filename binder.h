#ifndef __BINDER_H__
#define __BINDER_H__

#include <unistd.h>
#include <set>
#include <map>
#include <deque>
#include "sock.h"
#include "util.h"

class BINDER_SOCK: public SOCK {
    std::map<FUNC_SIGNATURE, std::deque<LOCATION>> funcmap;
    std::map<int, LOCATION> servermap;
public:
    BINDER_SOCK(int portnum);
    int handle_request(int i);
    int registerLocation(FUNC_SIGNATURE signature, LOCATION location);
    int getLocation(FUNC_SIGNATURE signature, LOCATION *location);
    int removeLocation(int sock_fd);
};

class BINDER {
    BINDER_SOCK *binder_sock;
public:
    BINDER(int portnum);
    void run();
    char* getHostName();
    int getPort();
};

#endif
