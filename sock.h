#ifndef __SOCK_H__
#define __SOCK_H__

#include <sys/socket.h>

#define MAX_CONNS      SOMAXCONN
#define ECONNOVERFLOW  -30
#define CLOSED         -1

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
    pthread_mutex_t mutex_conn;
    int init_socks();
    int handle_sock();
    int accept_socks();
public:
    SOCK(int portnum);
    virtual ~SOCK();
    virtual int handle_request(int sock_fd);
    int add_sockfd(int sockfd);
    void close_sockfd(int i);
    char *getHostName();
    int getPort();
    int getSockfd();
    int getError();
    int run();
    void terminate();
    bool terminated();
};

#endif
