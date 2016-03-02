#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>

#define MAX_CLIENTS 5

using namespace std;

int connections[5];
fd_set sock_fds;
int sock_fd, highsock_fd;

void error(string msg) {
    cerr << msg << endl;
    exit(1);
}

void copy_int_to_buf(int n, char *buf) {
    stringstream ss;
    ss << n;
    memcpy(buf, ss.str().c_str(), sizeof(int));
}

void capitalize(char *str, int len) {
    str[0] = toupper(str[0]);

    for (int i = 1; i < len; i++) {
        if (str[i-1] == ' ') str[i] = toupper(str[i]);
        else str[i] = tolower(str[i]);
    }

}

void init_socks() {
    FD_ZERO(&sock_fds);
    FD_SET(sock_fd, &sock_fds);
    highsock_fd = sock_fd;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connections[i] != 0) {
            FD_SET(connections[i], &sock_fds);
            if (connections[i] > highsock_fd) highsock_fd = connections[i];
        }
    }
}

void handle_sock() {
    int conn_sock_fd;

    conn_sock_fd = accept(sock_fd, NULL, NULL);

    if (conn_sock_fd >= 0) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (connections[i] == 0) {
                connections[i] = conn_sock_fd;
                break;
            }
        }
    }
}

void handle_request(int i) {
    char *buf, *msg;
    char tmpbuf[4];
    int msglen, buflen;
    int sock_fd = connections[i];

    if (recv(sock_fd, tmpbuf, sizeof(tmpbuf), 0) <= 0) {
        close(sock_fd);
        connections[i] = 0;
        return;
    }

    msglen = atoi(tmpbuf);
    buflen = msglen + 4;
    msg = new char[msglen];
    buf = new char[buflen];

    recv(sock_fd, msg, msglen, 0);

    capitalize(msg, msglen);

    copy_int_to_buf(msglen, buf);
    memcpy(buf+4, msg, msglen);

    send(sock_fd, buf, buflen, 0);

    delete [] msg;
    delete [] buf;
}

void accept_socks() {
    if (FD_ISSET(sock_fd, &sock_fds)) handle_sock();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (FD_ISSET(connections[i], &sock_fds)) handle_request(i);
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, tmp_addr;
    socklen_t socklen = sizeof(server_addr);
    char hostname[256];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) error("ERROR opening socket");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(0);

    if (bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) error("ERROR binding socket");

    getsockname(sock_fd, (struct sockaddr *) &tmp_addr, &socklen);
    gethostname(hostname, sizeof(hostname));

    cout << "SERVER_ADDRESS " << hostname << endl;
    cout << "SERVER_PORT " << ntohs(tmp_addr.sin_port) << endl;

    listen(sock_fd, MAX_CLIENTS);

    while (1) {
        init_socks();

        select(highsock_fd + 1, &sock_fds, NULL, NULL, NULL);

        accept_socks();
    }

    close(sock_fd);
}
