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
#include "binder.h"
#include "protocol.h"

using namespace std;

static map<FUNC_SIGNATURE, vector<LOCATION>> funcmap;

BINDER_SOCK::BINDER_SOCK(int portnum): SOCK(portnum) {}

int BINDER_SOCK::handle_request(int i) {
    INFO("in BINDER_SOCK handle_request");
    int sock_fd = connections[i];

    // receive a request from either server or client
    SEGMENT *segment = NULL;
    if (recvSegment(sock_fd, &segment) < 0) {
        INFO("connection to be closed");
        // todo: remove a server with this sock_fd
        close(sock_fd);
        connections[i] = 0;
        return RETURN_SUCCESS;
    }

    INFO("message received");

    FUNC_SIGNATURE func_signature = FUNC_SIGNATURE(NULL, NULL);
    LOCATION location = LOCATION(NULL, 0);
    map<FUNC_SIGNATURE, vector<LOCATION>>::iterator it;
    REQ_REG_MESSAGE *req_reg_message = NULL;
    MESSAGE *res_reg_success_message = NULL;
    REQ_LOC_MESSAGE *req_loc_message = NULL;
    MESSAGE *res_loc_success_message = NULL;
    MESSAGE *res_failure_message = NULL;
    SEGMENT *res_reg_success_segment = NULL;
    SEGMENT *res_loc_success_segment = NULL;
    SEGMENT *res_failure_segment = NULL;
    vector<LOCATION> locations;

    // handle the request according to its type
    switch (segment->type) {

        case REQUEST_REGISTER:
            INFO("register request message");
            // handle register request
            req_reg_message = dynamic_cast<REQ_REG_MESSAGE*>(segment->message);
            func_signature = FUNC_SIGNATURE(req_reg_message->name, req_reg_message->argTypes);
            location = LOCATION(req_reg_message->serverID, req_reg_message->port);
            it = funcmap.find(func_signature);

            if (it == funcmap.end()) {
                locations.push_back(location);
                funcmap.insert(pair<FUNC_SIGNATURE, vector<LOCATION>>(func_signature, locations));
            } else {
                locations = it->second;

                for (unsigned int i = 0; i < locations.size(); i++) {
                    if (location == locations[i]) {
                        // send a register failure response
                        res_failure_message = new RES_FAILURE_MESSAGE(WDUPREG);
                        res_failure_segment = new SEGMENT(REGISTER_FAILURE, res_failure_message);
                        sendSegment(sock_fd, res_failure_segment);
                        return RETURN_SUCCESS;
                    }
                }
            }

            // send a register success response
            res_reg_success_message = new RES_REG_SUCCESS_MESSAGE(RETURN_SUCCESS);
            res_reg_success_segment = new SEGMENT(REGISTER_SUCCESS, res_reg_success_message);
            sendSegment(sock_fd, res_reg_success_segment);

            break;

        case REQUEST_LOCATION:
            INFO("location request message");
            // handle location request
            req_loc_message = dynamic_cast<REQ_LOC_MESSAGE*>(segment->message);
            func_signature = FUNC_SIGNATURE(req_loc_message->name, req_loc_message->argTypes);
            it = funcmap.find(func_signature);

            if (it == funcmap.end()) {
                // send a location failure response
                res_failure_message = new RES_FAILURE_MESSAGE(ENOLOCATION);
                res_failure_segment = new SEGMENT(LOCATION_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
            } else {
                // send a location success response
                locations = it->second;
                location = locations[0];
                // todo: implement round-robin location selection
                res_loc_success_message = new RES_LOC_SUCCESS_MESSAGE(location.hostname, location.port);
                res_loc_success_segment = new SEGMENT(LOCATION_SUCCESS, res_loc_success_message);
                sendSegment(sock_fd, res_loc_success_segment);
            }

            break;

        default: return EUNKNOWN;
    }

    return RETURN_SUCCESS;

}

int main(int argc, char *argv[]) {
    // create a binder socket for server and client connections
    BINDER_SOCK *sock_binder = new BINDER_SOCK(0);
    cout << "BINDER_ADDRESS " << sock_binder->getHostName() << endl;
    cout << "BINDER_PORT " << sock_binder->getPort() << endl;
    sock_binder->run();
}
