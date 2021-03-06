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
#include <vector>
#include "binder.h"
#include "protocol.h"

using namespace std;

BINDER_SOCK::BINDER_SOCK(int portnum): SOCK(portnum) {}

int BINDER_SOCK::registerLocation(int sock_fd, FUNC_SIGNATURE &signature, LOCATION &location) {
    deque<LOCATION> locations;
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator it;

    it = funcmap.find(signature);

    if (it == funcmap.end()) {

        locations.push_back(location);
        funcmap.insert(pair<FUNC_SIGNATURE, deque<LOCATION>>(signature, locations));

    } else {

        locations = it->second;

        for (unsigned int i = 0; i < locations.size(); i++) {
            if (location == locations[i]) return EDUPLOCATION;
        }

        locations.push_back(location);
        it->second = locations;
    }

    servermap.insert(pair<int, LOCATION>(sock_fd, location));

    return RETURN_SUCCESS;
}

int BINDER_SOCK::getLocation(FUNC_SIGNATURE &signature, LOCATION &location) {
    deque<LOCATION> locations;
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator it;

    it = funcmap.find(signature);

    if (it == funcmap.end()) return ENOLOCATION;

    locations = it->second;
    location = locations.front();

    for (it = funcmap.begin(); it != funcmap.end(); it++) {
        locations = it->second;

        for (unsigned int i = 0; i < locations.size(); i++) {
            if (locations[i] == location) {
                locations.erase(locations.begin() + i);
                locations.push_back(location);
                it->second = locations;
            }
        }
    }

    return RETURN_SUCCESS;
}

int BINDER_SOCK::removeLocation(int sock_fd) {
    map<FUNC_SIGNATURE, deque<LOCATION>>::iterator fit;
    map<int, LOCATION>::iterator sit;
    deque<LOCATION>::iterator lit;
    deque<LOCATION> locations;

    sit = servermap.find(sock_fd);

    if (sit == servermap.end()) return WNOLOCATION;

    LOCATION location = sit->second;
    vector<map<FUNC_SIGNATURE, deque<LOCATION>>::iterator> vits;

    for (fit = funcmap.begin(); fit != funcmap.end(); fit++) {
        locations = fit->second;

        for (lit = locations.begin(); lit != locations.end(); lit++) {
            if (location == *lit) {
                locations.erase(lit);
                fit->second = locations;
                vits.push_back(fit);
                break;
            }
        }
    }

    for (unsigned int i = 0; i < vits.size(); i++) {
        locations = vits[i]->second;
        if (locations.size() == 0) funcmap.erase(vits[i]);
    }

    servermap.erase(sock_fd);

    return RETURN_SUCCESS;
}

int BINDER_SOCK::terminateLocations(SEGMENT *segment) {
    for (map<int, LOCATION>::iterator it = servermap.begin(); it != servermap.end(); it++) {
        sendSegment(it->first, segment);
    }
    return RETURN_SUCCESS;
}

int BINDER_SOCK::handle_request(int sock_fd) {
    int ret = RETURN_SUCCESS;

    // receive a request from either server or client
    SEGMENT *segment = NULL;
    if (recvSegment(sock_fd, &segment) < 0) {
        close_sockfd(sock_fd);
        removeLocation(sock_fd);
        return ret;
    }

    // handle the request according to its type
    switch (segment->type) {

        case REQUEST_REGISTER:
        {
            REQ_REG_MESSAGE *req_reg_message = dynamic_cast<REQ_REG_MESSAGE*>(segment->message);
            FUNC_SIGNATURE func_signature(req_reg_message->name, req_reg_message->argTypes);
            LOCATION location(req_reg_message->serverID, req_reg_message->port);

            ret = registerLocation(sock_fd, func_signature, location);

            if (ret < RETURN_SUCCESS) {
                // send a register failure response
                MESSAGE *res_failure_message = new RES_FAILURE_MESSAGE(ret);
                SEGMENT *res_failure_segment = new SEGMENT(REGISTER_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
                delete res_failure_segment;
            } else {
                // send a register success response
                MESSAGE *res_reg_success_message = new RES_REG_SUCCESS_MESSAGE(ret);
                SEGMENT *res_reg_success_segment = new SEGMENT(REGISTER_SUCCESS, res_reg_success_message);
                sendSegment(sock_fd, res_reg_success_segment);
                delete res_reg_success_segment;
            }

        }

        break;

        case REQUEST_LOCATION:
        {
            REQ_LOC_MESSAGE *req_loc_message = dynamic_cast<REQ_LOC_MESSAGE*>(segment->message);
            LOCATION location;
            FUNC_SIGNATURE func_signature(req_loc_message->name, req_loc_message->argTypes);

            ret = getLocation(func_signature, location);

            if (ret < RETURN_SUCCESS) {
                // send a location failure response
                MESSAGE *res_failure_message = new RES_FAILURE_MESSAGE(ret);
                SEGMENT *res_failure_segment = new SEGMENT(LOCATION_FAILURE, res_failure_message);
                sendSegment(sock_fd, res_failure_segment);
                delete res_failure_segment;
            } else {
                // send a location success response
                MESSAGE *res_loc_success_message = new RES_LOC_SUCCESS_MESSAGE(location.hostname, location.port);
                SEGMENT *res_loc_success_segment = new SEGMENT(LOCATION_SUCCESS, res_loc_success_message);
                sendSegment(sock_fd, res_loc_success_segment);
                delete res_loc_success_segment;
            }
        }

        break;

        case REQUEST_TERMINATE:
        {
            MESSAGE *req_term_message = new REQ_TERM_MESSAGE();
            SEGMENT *req_term_segment = new SEGMENT(REQUEST_TERMINATE, req_term_message);

            terminateLocations(req_term_segment);

            delete req_term_segment;

            terminate();
        }

        break;

        default: return EUNKNOWN;
    }

    delete segment;

    return RETURN_SUCCESS;

}

BINDER::BINDER(int portnum) {
    binder_sock = new BINDER_SOCK(portnum);
}

BINDER::~BINDER() {
    if (binder_sock) delete binder_sock;
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
    delete binder;
}

void print(map<FUNC_SIGNATURE, std::deque<LOCATION>> funcmap) {
    map<FUNC_SIGNATURE, std::deque<LOCATION>>::iterator it;
    deque<LOCATION> locations;
    INFO("FUNCMAP:");
    for (it = funcmap.begin(); it != funcmap.end(); it++) {
        FUNC_SIGNATURE s = it->first;
        locations = it->second;
        cout << s.name << ": ";
        for (unsigned int i = 0; i < locations.size(); i++) {
            cout << "[" << locations[i].hostname << ":" << locations[i].port << "]" << ", ";
        }
        cout << endl;
    }
}

void print(map<int, LOCATION> servermap) {
    map<int, LOCATION>::iterator it;
    INFO("SERVERMAP:");
    for (it = servermap.begin(); it != servermap.end(); it++) {
        int s = it->first;
        LOCATION location = it->second;
        cout << s << ":" << "[" << location.hostname << ":" << location.port << "]" << endl;
    }
}
