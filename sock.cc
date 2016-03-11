#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "sock.h"
#include "util.h"
#include "scheduler.h"

using namespace std;

SOCK::SOCK(int portnum): TERMINATED(false) {
    struct sockaddr_in sock_addr, tmp_addr;
    socklen_t socklen = sizeof(sock_addr);

    error = RETURN_SUCCESS;
    hostname = new char[MAX_SERVER_NAME_LEN];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) {
        error = ESOCKET;
        return;
    }

    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = INADDR_ANY;
    sock_addr.sin_port = htons(portnum);

    if (bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
        error = EBIND;
        return;
    }

    getsockname(sock_fd, (struct sockaddr *) &tmp_addr, &socklen);
    gethostname(hostname, sizeof(hostname));

    port = ntohs(tmp_addr.sin_port);

    for (int i = 0; i < MAX_CONNS; i++) {
        connections[i] = CLOSED;
    }

    scheduler = new SCHEDULER(this);
    pthread_mutex_init(&mutex_conn, NULL);
}

SOCK::~SOCK() {
    if (hostname) delete [] hostname;
    if (scheduler) delete scheduler;
}

int SOCK::init_socks() {
    FD_ZERO(&sock_fds);
    FD_SET(sock_fd, &sock_fds);
    highsock_fd = sock_fd;

    pthread_mutex_lock(&mutex_conn);
    for (int i = 0; i < MAX_CONNS; i++) {
        if (connections[i] != CLOSED) {
            FD_SET(connections[i], &sock_fds);
            if (connections[i] > highsock_fd) highsock_fd = connections[i];
        }
    }
    pthread_mutex_unlock(&mutex_conn);

    return RETURN_SUCCESS;
}

int SOCK::handle_sock() {
    int conn_sock_fd;

    conn_sock_fd = accept(sock_fd, NULL, NULL);

    if (conn_sock_fd >= 0) {
        pthread_mutex_lock(&mutex_conn);
        for (int i = 0; i < MAX_CONNS; i++) {
            if (connections[i] == CLOSED) {
                connections[i] = conn_sock_fd;
                break;
            }
        }
        pthread_mutex_unlock(&mutex_conn);
    } else {
        return EACCEPT;
    }

    return RETURN_SUCCESS;
}

int SOCK::handle_request(int sock_fd) { return RETURN_SUCCESS; }

int SOCK::accept_socks() {
    if (FD_ISSET(sock_fd, &sock_fds)) handle_sock();

    pthread_mutex_lock(&mutex_conn);
    for (int i = 0; i < MAX_CONNS; i++) {
        if (FD_ISSET(connections[i], &sock_fds)) scheduler->add_job(connections[i]);
    }
    pthread_mutex_unlock(&mutex_conn);

    return RETURN_SUCCESS;
}

void* SOCK::run_scheduler(void *ptr) {
    SCHEDULER *scheduler = (SCHEDULER *) ptr;
    scheduler->run();
    return NULL;
}

int SOCK::run() {
    listen(sock_fd, MAX_CONNS);

    pthread_t scheduler_thread;
    if(pthread_create(&scheduler_thread, NULL, run_scheduler, (void *) scheduler)) return ETHREAD;

    while (!TERMINATED) {
        init_socks();

        select(highsock_fd + 1, &sock_fds, NULL, NULL, NULL);

        accept_socks();
    }

    close(sock_fd);

    scheduler->terminate();

    return RETURN_SUCCESS;
}

void SOCK::terminate() { TERMINATED = true; }

bool SOCK::terminated() { return TERMINATED; }

int SOCK::add_sockfd(int sockfd) {
    pthread_mutex_lock(&mutex_conn);
    for (int i = 0; i < MAX_CONNS; i++) {
        if (connections[i] == 0) {
            connections[i] = sockfd;
            pthread_mutex_unlock(&mutex_conn);
            return RETURN_SUCCESS;
        }
    }

    pthread_mutex_unlock(&mutex_conn);

    return ECONNOVERFLOW;
}

void SOCK::close_sockfd(int sockfd) {
    pthread_mutex_lock(&mutex_conn);
    for (int i = 0; i < MAX_CONNS; i++) {
        if (connections[i] == sockfd) {
            connections[i] = 0;
            break;
        }
    }
    scheduler->remove_job(sockfd);
    close(sockfd);
    pthread_mutex_unlock(&mutex_conn);
}

char* SOCK::getHostName() { return hostname; }
int SOCK::getPort() { return port; }
int SOCK::getSockfd() { return sock_fd; }
int SOCK::getError() { return error; }
