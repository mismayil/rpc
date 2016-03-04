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
#include "util.h"
#include "sock.h"

using namespace std;

SERVER_BINDER_SOCK *sock_binder;
SERVER_CLIENT_SOCK *sock_client;

SERVER_BINDER_SOCK::SERVER_BINDER_SOCK(int portnum): SOCK(portnum) {}
SERVER_CLIENT_SOCK::SERVER_CLIENT_SOCK(int portnum): SOCK(portnum) {}

int SERVER_BINDER_SOCK::handle_request(int i) {

}

int SERVER_CLIENT_SOCK::handle_request(int i) {

}

void *handle_binder(void *args) {
    sock_binder->run();
    return NULL;
}

void *handle_clients(void *args) {
    sock_client->run();
    return NULL;
}

int rpcInit() {
    pthread_t thread_binder;
    int error;

    sock_binder = new SERVER_BINDER_SOCK(0);
    error = sock_binder->getError();
    if (error) return error;

    sock_client = new SERVER_CLIENT_SOCK(0);
    error = sock_client->getError();
    if (error) return error;

    if (pthread_create(&thread_binder, NULL, &handle_binder, NULL)) return ETHREAD;

    return RETURN_SUCCESS;
}

int rpcCall(char* name, int* argTypes, void** args) {
    char *BINDER_ADDRESS, *SERVER_ADDRESS;
    int BINDER_PORT, SERVER_PORT;
    int binder_sock_fd, server_sock_fd;
    RES_FAILURE_MESSAGE *res_failure_message;

    // get binder address and port
    BINDER_ADDRESS = getenv("BINDER_ADDRESS");
    BINDER_PORT = atoi(getenv("BINDER_PORT"));

    // connect to the binder
    binder_sock_fd = connectTo(BINDER_ADDRESS, BINDER_PORT);
    if (binder_sock_fd < 0) return binder_sock_fd;

    // send a location request to the binder
    MESSAGE *req_loc_message = new REQ_LOC_MESSAGE(name, argTypes);
    SEGMENT *req_loc_segment = new SEGMENT(REQUEST_LOCATION, req_loc_message);
    sendSegment(binder_sock_fd, req_loc_segment);

    // receive location response from the binder
    SEGMENT *res_loc_segment = recvSegment(binder_sock_fd);
    MESSAGE *res_loc_message = res_loc_segment->message;
    RES_LOC_SUCCESS_MESSAGE *res_loc_success_message;

    switch (res_loc_segment->type) {
        case LOCATION_SUCCESS:
            res_loc_success_message = dynamic_cast<RES_LOC_SUCCESS_MESSAGE*>(res_loc_message);
            SERVER_ADDRESS = res_loc_success_message->serverID;
            SERVER_PORT = res_loc_success_message->port;
            break;
        case LOCATION_FAILURE:
            res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_loc_message);
            return res_failure_message->reasonCode;
        default: return EUNKNOWN;
    }

    // connect to the server
    server_sock_fd = connectTo(SERVER_ADDRESS, SERVER_PORT);
    if (server_sock_fd < 0) return server_sock_fd;

    // send an execute request to the server
    MESSAGE *req_execute_message = new REQ_EXEC_MESSAGE(name, argTypes, args);
    SEGMENT *req_exec_segment = new SEGMENT(REQUEST_EXECUTE, req_execute_message);
    sendSegment(server_sock_fd, req_exec_segment);

    // receive an execute response from the server
    SEGMENT *res_exec_segment = recvSegment(server_sock_fd);
    MESSAGE *res_exec_message = res_exec_segment->message;
    RES_EXEC_SUCCESS_MESSAGE *res_exec_success_message;

    switch (res_exec_segment->type) {
        case EXECUTE_SUCCESS:
            res_exec_success_message = dynamic_cast<RES_EXEC_SUCCESS_MESSAGE*>(res_exec_message);
            name = res_exec_success_message->name;
            argTypes = res_exec_success_message->argTypes;
            args = res_exec_success_message->args;
            break;
        case EXECUTE_FAILURE:
            res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_loc_message);
            return res_failure_message->reasonCode;
        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;
}

int rpcCacheCall(char* name, int* argTypes, void** args);

int rpcRegister(char* name, int* argTypes, skeleton f) {
    char *serverID = sock_binder->getHostName();
    int port = sock_binder->getPort();
    int binder_sock_fd = sock_binder->getSockfd();

    // send a register request to the binder
    MESSAGE *req_reg_message = new REQ_REG_MESSAGE(serverID, port, name, argTypes);
    SEGMENT *req_reg_segment = new SEGMENT(REQUEST_REGISTER, req_reg_message);
    sendSegment(binder_sock_fd, req_reg_segment);

    // receive register response from the binder
    SEGMENT* res_reg_segment = recvSegment(binder_sock_fd);
    MESSAGE *res_reg_message = res_reg_segment->message;
    RES_REG_SUCCESS_MESSAGE *res_reg_success_message;
    RES_FAILURE_MESSAGE *res_failure_message;

    switch (res_reg_segment->type) {
        case REGISTER_SUCCESS:
            res_reg_success_message = dynamic_cast<RES_REG_SUCCESS_MESSAGE*>(res_reg_message);
            return res_reg_success_message->reasonCode;
        case REGISTER_FAILURE:
            res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_reg_message);
            return res_failure_message->reasonCode;
        default: return EUNKNOWN;
    }
}

int rpcExecute() {
    pthread_t thread_client;

    if (pthread_create(&thread_client, NULL, &handle_clients, NULL)) return ETHREAD;

    return RETURN_SUCCESS;
}

int rpcTerminate();
