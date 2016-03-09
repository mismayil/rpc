#ifndef __BINDER_H__
#define __BINDER_H__

#include <unistd.h>
#include <set>
#include <map>
#include <vector>
#include "sock.h"
#include "util.h"

class BINDER_SOCK: public SOCK {
    std::map<FUNC_SIGNATURE, std::vector<LOCATION>> funcmap;
    std::set<int> server_sock_fds;
public:
    BINDER_SOCK(int portnum);
    int handle_request(int i);
    int registerLocation(FUNC_SIGNATURE signature, LOCATION location);
    int getLocation(FUNC_SIGNATURE signature, LOCATION *location);
};

class BINDER {
    BINDER_SOCK *sock_binder;
public:
    BINDER(int portnum);
    void run();
    char* getHostName();
    int getPort();
};

#endif
