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
            SERVER_ADDRESS = res_loc_success_message->server;
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

int rpcRegister(char* name, int* argTypes, skeleton f);

int rpcExecute();

int rpcTerminate();
