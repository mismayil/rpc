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
#include "scheduler.h"

using namespace std;

static int BINDER_SOCK_FD;
static SERVER_SOCK *server_sock;

SERVER_SOCK::SERVER_SOCK(int portnum): SOCK(portnum) {}

int SERVER_SOCK::registerFunction(FUNC_SIGNATURE &signature, skeleton f) {
    map<FUNC_SIGNATURE, skeleton>::iterator it;
    it = funcmap.find(signature);
    if (it == funcmap.end()) funcmap.insert(pair<FUNC_SIGNATURE, skeleton>(signature, f));
    else it->second = f;
    return RETURN_SUCCESS;
}

int SERVER_SOCK::executeFunction(FUNC_SIGNATURE &signature, int *argTypes, void **args) {
    map<FUNC_SIGNATURE, skeleton>::iterator it;
    it = funcmap.find(signature);

    // function not found
    if (it == funcmap.end()) return ENOFUNCTION;

    // execute the function
    skeleton func = it->second;
    int ret = func(argTypes, args);
    return ret;
}

int SERVER_SOCK::handle_request(int sock_fd) {
    int ret = RETURN_SUCCESS;

    // receive a request from either binder or client
    SEGMENT *segment = NULL;
    if (recvSegment(sock_fd, &segment) < 0) {
        close_sockfd(sock_fd);
        return ret;
    }
    
    switch (segment->type) {

        case REQUEST_EXECUTE:
        {
            REQ_EXEC_MESSAGE *req_exec_message = dynamic_cast<REQ_EXEC_MESSAGE*>(segment->message);
            FUNC_SIGNATURE func_signature(req_exec_message->name, req_exec_message->argTypes);
            ret = executeFunction(func_signature, req_exec_message->argTypes, req_exec_message->args);

            if (ret < RETURN_SUCCESS) {
                // send an execute failure response to the client
                MESSAGE *res_failure_message = new RES_FAILURE_MESSAGE(ret);
                SEGMENT *res_failure_segment = new SEGMENT(EXECUTE_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
                delete res_failure_segment;
            } else {
                // send an execute success response to the client
                MESSAGE *res_exec_success_message = new RES_EXEC_SUCCESS_MESSAGE(req_exec_message->name, req_exec_message->argTypes, req_exec_message->args);
                SEGMENT *res_exec_success_segment = new SEGMENT(EXECUTE_SUCCESS, res_exec_success_message);
                sendSegment(sock_fd, res_exec_success_segment);
                delete res_exec_success_segment;
            }
        }

        break;

        case REQUEST_TERMINATE:
        {
            // verify binder socket
            if (sock_fd != BINDER_SOCK_FD) return EIBINDER;

            // terminate server
            terminate();
        }

        break;

        default: return EUNKNOWN;
    }

    delete segment;

    return RETURN_SUCCESS;
}

int rpcInit() {
    char *BINDER_ADDRESS;
    int BINDER_PORT;
    int ret = RETURN_SUCCESS;

    // create a server socket
    server_sock = new SERVER_SOCK(0);
    ret = server_sock->getError();
    if (ret) return ret;

    // get binder address and port
    BINDER_ADDRESS = getenv("BINDER_ADDRESS");
    BINDER_PORT = atoi(getenv("BINDER_PORT"));

    // connect to the binder
    BINDER_SOCK_FD = connectTo(BINDER_ADDRESS, BINDER_PORT);
    if (BINDER_SOCK_FD < 0) return BINDER_SOCK_FD;

    server_sock->add_sockfd(BINDER_SOCK_FD);

    return ret;
}

int rpcCall(char* name, int* argTypes, void** args) {
    char *BINDER_ADDRESS, *SERVER_ADDRESS;
    int BINDER_PORT, SERVER_PORT;
    int binder_sock_fd, server_sock_fd;
    int ret = RETURN_SUCCESS;

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

    delete req_loc_segment;

    // receive a location response from the binder
    SEGMENT *res_loc_segment = NULL;
    ret = recvSegment(binder_sock_fd, &res_loc_segment);
    close(binder_sock_fd);
    if (ret) return ret;

    MESSAGE *res_loc_message = res_loc_segment->message;

    switch (res_loc_segment->type) {
        case LOCATION_SUCCESS:
        {
            RES_LOC_SUCCESS_MESSAGE *res_loc_success_message = dynamic_cast<RES_LOC_SUCCESS_MESSAGE*>(res_loc_message);
            SERVER_ADDRESS = res_loc_success_message->serverID;
            SERVER_PORT = res_loc_success_message->port;
        }
        break;
        case LOCATION_FAILURE:
        {
            RES_FAILURE_MESSAGE *res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_loc_message);
            ret = res_failure_message->reasonCode;
        }
        break;
        default: return EUNKNOWN;
    }

    delete res_loc_segment;

    if (ret) return ret;

    // connect to the server
    server_sock_fd = connectTo(SERVER_ADDRESS, SERVER_PORT);
    delete SERVER_ADDRESS;
    if (server_sock_fd < 0) return server_sock_fd;

    // send an execute request to the server
    MESSAGE *req_execute_message = new REQ_EXEC_MESSAGE(name, argTypes, args);
    SEGMENT *req_exec_segment = new SEGMENT(REQUEST_EXECUTE, req_execute_message);
    sendSegment(server_sock_fd, req_exec_segment);

    delete req_exec_segment;

    // receive an execute response from the server
    SEGMENT *res_exec_segment = NULL;
    ret = recvSegment(server_sock_fd, &res_exec_segment);
    close(server_sock_fd);
    if (ret) return ret;

    MESSAGE *res_exec_message = res_exec_segment->message;

    switch (res_exec_segment->type) {

        case EXECUTE_SUCCESS:
        {
            RES_EXEC_SUCCESS_MESSAGE *res_exec_success_message = dynamic_cast<RES_EXEC_SUCCESS_MESSAGE*>(res_exec_message);

            int argc = 1;
            while (res_exec_success_message->argTypes[argc-1] != ARG_TERMINATOR) {
                argc++;
            }

            for (int i = 0; i < argc; i++) {
                args[i] = res_exec_success_message->args[i];
            }
        }

        break;

        case EXECUTE_FAILURE:
        {
            RES_FAILURE_MESSAGE *res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_exec_message);
            ret = res_failure_message->reasonCode;
        }

        break;

        default: return EUNKNOWN;
    }

    delete res_exec_segment;

    return ret;
}

int rpcCacheCall(char* name, int* argTypes, void** args);

int rpcRegister(char* name, int* argTypes, skeleton f) {
    int ret = RETURN_SUCCESS;

    if (server_sock == NULL || server_sock->getError()) return ESOCKET;

    // send a register request to the binder
    MESSAGE *req_reg_message = new REQ_REG_MESSAGE(server_sock->getHostName(), server_sock->getPort(), name, argTypes);
    SEGMENT *req_reg_segment = new SEGMENT(REQUEST_REGISTER, req_reg_message);
    sendSegment(BINDER_SOCK_FD, req_reg_segment);

    delete req_reg_segment;

    // receive a register response from the binder
    SEGMENT* res_reg_segment = NULL;
    ret = recvSegment(BINDER_SOCK_FD, &res_reg_segment);
    if (ret) return ret;

    MESSAGE *res_reg_message = res_reg_segment->message;

    switch (res_reg_segment->type) {

        case REGISTER_SUCCESS:
        {
            RES_REG_SUCCESS_MESSAGE *res_reg_success_message = dynamic_cast<RES_REG_SUCCESS_MESSAGE*>(res_reg_message);
            FUNC_SIGNATURE func_signature(name, argTypes);
            server_sock->registerFunction(func_signature, f);
            ret = res_reg_success_message->reasonCode;
        }

        break;

        case REGISTER_FAILURE:
        {
            RES_FAILURE_MESSAGE *res_failure_message = dynamic_cast<RES_FAILURE_MESSAGE*>(res_reg_message);
            ret = res_failure_message->reasonCode;
        }

        break;

        default: return EUNKNOWN;
    }

    delete res_reg_segment;

    return ret;
}

int rpcExecute() {
    if (server_sock == NULL || server_sock->getError()) return ESOCKET;
    server_sock->run();
    delete server_sock;
    return RETURN_SUCCESS;
}

int rpcTerminate() {
    char *BINDER_ADDRESS;
    int BINDER_PORT;
    int binder_sock_fd;
    int ret = RETURN_SUCCESS;

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

    delete req_term_segment;

    close(binder_sock_fd);

    return ret;
}
