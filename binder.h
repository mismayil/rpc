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
    int registerLocation(int sock_fd, FUNC_SIGNATURE &signature, LOCATION &location);
    int getLocation(FUNC_SIGNATURE &signature, LOCATION &location);
    int removeLocation(int sock_fd);
    int terminateLocations(SEGMENT *segment);
};

class BINDER {
    BINDER_SOCK *binder_sock;
public:
    BINDER(int portnum);
    ~BINDER();
    void run();
    char* getHostName();
    int getPort();
};

void print(std::map<FUNC_SIGNATURE, std::deque<LOCATION>> funcmap);
void print(std::map<int, LOCATION> servermap);
#endif
