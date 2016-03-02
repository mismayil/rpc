#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "rpc.h"
#include "protocol.h"

using namespace std;

int rpcInit();

int rpcCall(char* name, int* argTypes, void** args) {
    char *BINDER_ADDRESS;
    int BINDER_PORT;
    int sock_fd;
    struct sockaddr_in binder_addr;
    struct hostent *binder;
    char *buf;

    // get binder address and port
    BINDER_ADDRESS = getenv("BINDER_ADDRESS");
    BINDER_PORT = atoi(getenv("BINDER_PORT"));

    // open a socket for binder connection
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return ESOCKET;

    // clear binder address
    memset(&binder_addr, 0, sizeof(binder_addr));

    // get binder address info
    binder = gethostbyname(BINDER_ADDRESS);
    if (!binder) return ENOBINDER;

    // populate binder address
    binder_addr.sin_family = AF_INET;
    memcpy(&binder_addr.sin_addr.s_addr, binder->h_addr, binder->h_length);
    binder_addr.sin_port = htons(BINDER_PORT);

    // connect to the binder
    if (connect(sock_fd, (struct sockaddr *) &binder_addr, sizeof(binder_addr)) < 0) return ECONNECT;

    // send a location request to the binder
    return 0;

}

int rpcCacheCall(char* name, int* argTypes, void** args);

int rpcRegister(char* name, int* argTypes, skeleton f);

int rpcExecute();

int rpcTerminate();
