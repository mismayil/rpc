#ifndef __SOCK_H__
#define __SOCK_H__

#include <sys/socket.h>
#include <deque>
#include <pthread.h>

#define MAX_CONNS      SOMAXCONN
#define ECONNOVERFLOW  -30
#define NUM_THREADS    8

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
    pthread_t threads[NUM_THREADS];
    std::deque<int> jobs;
    pthread_cond_t cond_jobs;
    pthread_mutex_t mutex_jobs;
    void add_job(int i);
    static void *execute_job(void *args);
    int init_socks();
    int handle_sock();
    int accept_socks();
public:
    SOCK(int portnum);
    virtual ~SOCK();
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
