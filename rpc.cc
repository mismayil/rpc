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
#include <map>
#include "rpc.h"
#include "protocol.h"
#include "util.h"
#include "sock.h"

using namespace std;

static int binder_sock_fd;
static SERVER_SOCK *server_sock;
static pthread_t thread_server;
static map<FUNC_SIGNATURE, skeleton> funcmap;

SERVER_SOCK::SERVER_SOCK(int portnum): SOCK(portnum) {}

// handle binder and client requests
int SERVER_SOCK::handle_request(int i) {
    INFO("in SERVER_SOCK handle_request");
    int sock_fd = connections[i];

    // receive a request from either binder or client
    SEGMENT *segment = NULL;
    if (recvSegment(sock_fd, &segment) < 0) {
        close(sock_fd);
        connections[i] = 0;
        return RETURN_SUCCESS;
    }

    REQ_EXEC_MESSAGE *req_exec_message = NULL;
    MESSAGE *res_exec_success_message = NULL;
    MESSAGE *res_failure_message = NULL;
    SEGMENT *res_exec_success_segment = NULL;
    SEGMENT *res_failure_segment = NULL;

    map<FUNC_SIGNATURE, skeleton>::iterator it;
    FUNC_SIGNATURE func_signature = FUNC_SIGNATURE(NULL, NULL);

    switch (segment->type) {

        case REQUEST_EXECUTE:

            req_exec_message = dynamic_cast<REQ_EXEC_MESSAGE*>(segment->message);

            // check if this function already exists
            func_signature = FUNC_SIGNATURE(req_exec_message->name, req_exec_message->argTypes);
            it = funcmap.find(func_signature);

            if (it == funcmap.end()) {

                // send an execute failure response to the client
                res_failure_message = new RES_FAILURE_MESSAGE(ENOFUNCTION);
                res_failure_segment = new SEGMENT(EXECUTE_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);

            } else {

                // execute the function
                skeleton func = it->second;
                int res = func(req_exec_message->argTypes, req_exec_message->args);

                if (res < RETURN_SUCCESS) {
                    // send an execute failure response to the client
                    res_failure_message = new RES_FAILURE_MESSAGE(res);
                    res_failure_segment = new SEGMENT(EXECUTE_FAILURE, res_failure_message);
                    sendSegment(sock_fd, res_failure_segment);
                } else {
                    // send an execute success response to the client
                    res_exec_success_message = new RES_EXEC_SUCCESS_MESSAGE(req_exec_message->name, req_exec_message->argTypes, req_exec_message->args);
                    res_exec_success_segment = new SEGMENT(EXECUTE_SUCCESS, res_exec_success_message);
                    sendSegment(sock_fd, res_exec_success_segment);
                }
            }

            break;

        case REQUEST_TERMINATE:
            // verify binder socket
            if (sock_fd != binder_sock_fd) return EIBINDER;

            // terminate server
            terminate();

            break;

        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;
}

// run the server
void *run_server(void *args) {
    INFO("in run_server");
    server_sock->run();
    return NULL;
}

int rpcInit() {
    INFO("in rpcInit");
    char *BINDER_ADDRESS;
    int BINDER_PORT;
    int error;

    // get binder address and port
    BINDER_ADDRESS = getenv("BINDER_ADDRESS");
    BINDER_PORT = atoi(getenv("BINDER_PORT"));

    // connect to the binder
    binder_sock_fd = connectTo(BINDER_ADDRESS, BINDER_PORT);
    if (binder_sock_fd < 0) return binder_sock_fd;

    INFO("connected to the binder");

    // create a server socket
    server_sock = new SERVER_SOCK(0);
    error = server_sock->getError();
    if (error) return error;

    server_sock->add_sock_fd(binder_sock_fd);

    INFO("SERVER_SOCK created");

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

    // receive a location response from the binder
    SEGMENT *res_loc_segment = NULL;
    recvSegment(binder_sock_fd, &res_loc_segment);
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
    SEGMENT *res_exec_segment = NULL;
    recvSegment(server_sock_fd, &res_exec_segment);
    MESSAGE *res_exec_message = res_exec_segment->message;
    RES_EXEC_SUCCESS_MESSAGE *res_exec_success_message;
    int argc = 1;

    switch (res_exec_segment->type) {
        case EXECUTE_SUCCESS:
            res_exec_success_message = dynamic_cast<RES_EXEC_SUCCESS_MESSAGE*>(res_exec_message);

            while (res_exec_success_message->argTypes[argc-1] != ARG_TERMINATOR) {
                argc++;
            }

            for (int i = 0; i < argc; i++) {
                args[i] = res_exec_success_message->args[i];
            }

            break;
        case EXECUTE_FAILURE:
            res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_exec_message);
            return res_failure_message->reasonCode;
        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;
}

int rpcCacheCall(char* name, int* argTypes, void** args);

int rpcRegister(char* name, int* argTypes, skeleton f) {
    INFO("in rpcRegister");

    // send a register request to the binder
    MESSAGE *req_reg_message = new REQ_REG_MESSAGE(server_sock->getHostName(), server_sock->getPort(), name, argTypes);
    SEGMENT *req_reg_segment = new SEGMENT(REQUEST_REGISTER, req_reg_message);
    sendSegment(binder_sock_fd, req_reg_segment);
    DEBUG("server", server_sock->getHostName());
    DEBUG("server_port", server_sock->getPort());
    INFO("register request sent");

    // receive a register response from the binder
    SEGMENT* res_reg_segment = NULL;
    recvSegment(binder_sock_fd, &res_reg_segment);
    MESSAGE *res_reg_message = res_reg_segment->message;
    RES_REG_SUCCESS_MESSAGE *res_reg_success_message;
    RES_FAILURE_MESSAGE *res_failure_message;
    FUNC_SIGNATURE func_signature(name, argTypes);
    map<FUNC_SIGNATURE, skeleton>::iterator it;

    INFO("register response received");

    switch (res_reg_segment->type) {

        case REGISTER_SUCCESS:
            res_reg_success_message = dynamic_cast<RES_REG_SUCCESS_MESSAGE*>(res_reg_message);
            INFO("register success");
            // store an entry for this function skeleton
            it = funcmap.find(func_signature);
            if (it == funcmap.end()) funcmap.insert(pair<FUNC_SIGNATURE, skeleton>(func_signature, f));
            else it->second = f;

            return res_reg_success_message->reasonCode;

        case REGISTER_FAILURE:
            INFO("register failure");
            res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_reg_message);
            return res_failure_message->reasonCode;

        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;
}

int rpcExecute() {
    INFO("in rpcExecute");
    if (pthread_create(&thread_server, NULL, &run_server, NULL)) return ETHREAD;
    pthread_join(thread_server, NULL);
    return RETURN_SUCCESS;
}

int rpcTerminate() {
    char *BINDER_ADDRESS;
    int BINDER_PORT;
    int binder_sock_fd;

    // get binder address and port
    BINDER_ADDRESS = getenv("BINDER_ADDRESS");
    BINDER_PORT = atoi(getenv("BINDER_PORT"));

    // connect to the binder
    binder_sock_fd = connectTo(BINDER_ADDRESS, BINDER_PORT);
    if (binder_sock_fd < 0) return binder_sock_fd;

    // send a terminate request to the binder
    MESSAGE *req_term_message = new REQ_TERM_MESSAGE();
    SEGMENT *req_term_segment = new SEGMENT(REQUEST_TERMINATE, req_term_message);
    sendSegment(binder_sock_fd, req_term_segment);

    return RETURN_SUCCESS;
}
