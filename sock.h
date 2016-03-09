#ifndef __SOCK_H__
#define __SOCK_H__

#include <sys/socket.h>

#define MAX_CONNS      SOMAXCONN
#define ECONNOVERFLOW  -30

// socket class with select
class SOCK {
protected:
    char *hostname;
    int port;
    int connections[MAX_CONNS];
    fd_set sock_fds;
    int sock_fd, highsock_fd;
    int error;
    bool TERMINATED;
    int init_socks();
    int handle_sock();
    int accept_socks();
public:
    SOCK(int portnum);
    ~SOCK();
    virtual int handle_request(int i);
    int add_sock_fd(int sockfd);
    char *getHostName();
    int getPort();
    int getSockfd();
    int getError();
    int run();
    void terminate();
};

#endif
