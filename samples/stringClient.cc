#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <queue>
#include <pthread.h>

using namespace std;

pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
bool STOP = false;

void error(string msg) {
    cerr << msg << endl;
    exit(1);
}

void copy_int_to_buf(int n, char *buf) {
    stringstream ss;
    ss << n;
    memcpy(buf, ss.str().c_str(), sizeof(int));
}

void *read_msg(void *ptr) {
    string msg;
    queue<string> *messages = (queue<string> *) ptr;

    while (getline(cin, msg)) {
        pthread_mutex_lock(&msg_mutex);
        messages->push(msg);
        pthread_mutex_unlock(&msg_mutex);
    }

    STOP = true;
}

void *send_msg(void *ptr) {
    int sock_fd;
    struct sockaddr_in server_addr;
    char *SERVER_ADDRESS;
    int SERVER_PORT;
    struct hostent *server;
    char *buf, *msg;
    char tmpbuf[4];
    int msglen, buflen;
    string str;

    queue<string> *messages = (queue<string> *) ptr;

    SERVER_ADDRESS = getenv("SERVER_ADDRESS");
    SERVER_PORT = atoi(getenv("SERVER_PORT"));

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) error("ERROR opening socket");

    memset(&server_addr, 0, sizeof(server_addr));

    server = gethostbyname(SERVER_ADDRESS);

    if (!server) error("ERROR finding server");

    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) error("ERROR connecting to the server");

    while (1) {

        if (STOP && messages->empty()) break;

        pthread_mutex_lock(&msg_mutex);

        if (messages->empty()) {
            pthread_mutex_unlock(&msg_mutex);
            continue;
        }

        str = messages->front();
        messages->pop();

        msglen = str.length() + 1;
        buflen = msglen + 4;
        msg = new char[msglen];
        buf = new char[buflen];
        copy_int_to_buf(msglen, buf);
        memcpy(buf+4, str.c_str(), msglen);

        pthread_mutex_unlock(&msg_mutex);

        send(sock_fd, buf, buflen, 0);
        recv(sock_fd, tmpbuf, sizeof(tmpbuf), 0);
        recv(sock_fd, msg, msglen, 0);

        cout << "Server: " << msg << endl;

        delete [] msg;
        delete [] buf;

        sleep(2);
    }

    close(sock_fd);
}


int main(int argc, char *argv[]) {
    queue<string> messages;
    pthread_t thread_read, thread_send;

    if (pthread_create(&thread_read, NULL, &read_msg, &messages)) error("ERROR creating read thread");

    if (pthread_create(&thread_send, NULL, &send_msg, &messages)) error("ERROR creating send thread");

    pthread_join(thread_read, NULL);
    pthread_join(thread_send, NULL);
}
