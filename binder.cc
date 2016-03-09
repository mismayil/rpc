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
#include "binder.h"
#include "protocol.h"

using namespace std;

BINDER_SOCK::BINDER_SOCK(int portnum): SOCK(portnum) {}

int BINDER_SOCK::registerLocation(FUNC_SIGNATURE signature, LOCATION location) {
    deque<LOCATION> locations;
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator it;

    it = funcmap.find(signature);

    if (it == funcmap.end()) {

        locations.push_back(location);
        funcmap.insert(pair<FUNC_SIGNATURE, deque<LOCATION>>(signature, locations));

    } else {

        locations = it->second;

        for (unsigned int i = 0; i < locations.size(); i++) {
            if (location == locations[i]) return RETURN_FAILURE;
        }
    }

    return RETURN_SUCCESS;
}

int BINDER_SOCK::getLocation(FUNC_SIGNATURE signature, LOCATION *location) {
    deque<LOCATION> locations;
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator it;

    it = funcmap.find(signature);

    if (it == funcmap.end()) return ENOLOCATION;

    locations = it->second;
    *location = locations[0];
    // todo: implement round-robin location selection
    return RETURN_SUCCESS;
}

int BINDER_SOCK::removeLocation(int sock_fd) {
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator mit;
    map<int, LOCATION>::iterator it;
    deque<LOCATION>::iterator dit;
    deque<LOCATION> locations;
    LOCATION location = LOCATION(NULL, 0);
    bool found = false;

    it = servermap.find(sock_fd);

    if (it == servermap.end()) return WNOLOCATION;

    location = it->second;

    for (mit = funcmap.begin(); mit != funcmap.end(); mit++) {
        locations = mit->second;

        for (dit = locations.begin(); dit != locations.end(); dit++) {
            if (location == *dit) {
                locations.erase(dit);
                found = true;
                break;
            }
        }
    }

    if (found) servermap.erase(sock_fd);

    return RETURN_SUCCESS;
}

int BINDER_SOCK::handle_request(int i) {
    INFO("in BINDER_SOCK handle_request");
    int sock_fd = connections[i];
    int res;

    // receive a request from either server or client
    SEGMENT *segment = NULL;
    if (recvSegment(sock_fd, &segment) < 0) {
        INFO("connection to be closed");
        close(sock_fd);
        connections[i] = 0;
        removeLocation(sock_fd);
        return RETURN_SUCCESS;
    }

    INFO("message received");

    FUNC_SIGNATURE func_signature = FUNC_SIGNATURE(NULL, NULL);
    LOCATION location = LOCATION(NULL, 0);
    REQ_REG_MESSAGE *req_reg_message = NULL;
    MESSAGE *res_reg_success_message = NULL;
    REQ_LOC_MESSAGE *req_loc_message = NULL;
    MESSAGE *res_loc_success_message = NULL;
    MESSAGE *res_failure_message = NULL;
    MESSAGE *req_term_message = NULL;
    SEGMENT *res_reg_success_segment = NULL;
    SEGMENT *res_loc_success_segment = NULL;
    SEGMENT *res_failure_segment = NULL;
    SEGMENT *req_term_segment = NULL;

    // handle the request according to its type
    switch (segment->type) {

        case REQUEST_REGISTER:
            INFO("register request message");

            req_reg_message = dynamic_cast<REQ_REG_MESSAGE*>(segment->message);
            func_signature = FUNC_SIGNATURE(req_reg_message->name, req_reg_message->argTypes);
            location = LOCATION(req_reg_message->serverID, req_reg_message->port);

            res = registerLocation(func_signature, location);

            if (res < RETURN_SUCCESS) {
                // send a register failure response
                res_failure_message = new RES_FAILURE_MESSAGE(res);
                res_failure_segment = new SEGMENT(REGISTER_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
            } else {
                // send a register success response
                res_reg_success_message = new RES_REG_SUCCESS_MESSAGE(res);
                res_reg_success_segment = new SEGMENT(REGISTER_SUCCESS, res_reg_success_message);
                sendSegment(sock_fd, res_reg_success_segment);
            }

            servermap.insert(pair<int, LOCATION>(sock_fd, location));

            break;

        case REQUEST_LOCATION:
            INFO("location request message");
            // handle location request
            req_loc_message = dynamic_cast<REQ_LOC_MESSAGE*>(segment->message);
            func_signature = FUNC_SIGNATURE(req_loc_message->name, req_loc_message->argTypes);

            res = getLocation(func_signature, &location);

            if (res < RETURN_SUCCESS) {
                // send a location failure response
                res_failure_message = new RES_FAILURE_MESSAGE(res);
                res_failure_segment = new SEGMENT(LOCATION_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
            } else {
                // send a location success response
                res_loc_success_message = new RES_LOC_SUCCESS_MESSAGE(location.hostname, location.port);
                res_loc_success_segment = new SEGMENT(LOCATION_SUCCESS, res_loc_success_message);
                sendSegment(sock_fd, res_loc_success_segment);
            }

            break;

        case REQUEST_TERMINATE:
            INFO("terminate message");

            req_term_message = new REQ_TERM_MESSAGE();
            req_term_segment = new SEGMENT(REQUEST_TERMINATE, req_term_message);

            // send terminate request to all servers
            for (map<int, LOCATION>::iterator it = servermap.begin(); it != servermap.end(); it++) {
                sendSegment(it->first, req_term_segment);
            }

            terminate();
            break;

        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;

}

BINDER::BINDER(int portnum) {
    binder_sock = new BINDER_SOCK(portnum);
}

void BINDER::run() {
    binder_sock->run();
}

char* BINDER::getHostName() { return binder_sock->getHostName(); }

int BINDER::getPort() { return binder_sock->getPort(); }

int main(int argc, char *argv[]) {
    // create a binder for server and client connections
    BINDER *binder = new BINDER(0);
    cout << "BINDER_ADDRESS " << binder->getHostName() << endl;
    cout << "BINDER_PORT " << binder->getPort() << endl;
    binder->run();
}
