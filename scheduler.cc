#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "scheduler.h"
#include "util.h"
#include "sock.h"

using namespace std;

SCHEDULER::SCHEDULER(SOCK *sock): sock(sock) {
    pthread_mutex_init(&mutex_jobs, NULL);
    pthread_cond_init(&cond_jobs, NULL);
}

SCHEDULER::~SCHEDULER() {
    pthread_mutex_destroy(&mutex_jobs);
    pthread_cond_destroy(&cond_jobs);
}

int SCHEDULER::run() {

    // create worker threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, execute_job, (void *) this);
    }

    // join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return RETURN_SUCCESS;
}

void SCHEDULER::add_job(int sockfd) {
    pthread_mutex_lock(&mutex_jobs);
    DEBUG("ADD JOB", sockfd);
    for (unsigned int k = 0; k < jobs.size(); k++) {
        if (jobs[k] == sockfd) {
            jobs.erase(jobs.begin() + k);
            break;
        }
    }
    jobs.push_back(sockfd);
    INFO("JOBS:");
    for (unsigned int j = 0; j < jobs.size(); j++) {
        DEBUG("JOB", jobs[j]);
    }
    pthread_cond_broadcast(&cond_jobs);
    pthread_mutex_unlock(&mutex_jobs);
}

void* SCHEDULER::execute_job(void *ptr) {
    SCHEDULER *scheduler = (SCHEDULER *) ptr;

    while (1) {
        INFO("JOBS:");
        pthread_mutex_lock(&scheduler->mutex_jobs);
        for (unsigned int j = 0; j < scheduler->jobs.size(); j++) {
            DEBUG("JOB", scheduler->jobs[j]);
        }
        INFO("waiting for a job...");
        pthread_cond_wait(&scheduler->cond_jobs, &scheduler->mutex_jobs);
        int sockfd = scheduler->jobs.back();
        DEBUG("EXEC JOB", sockfd);
        scheduler->jobs.pop_back();
        pthread_mutex_unlock(&scheduler->mutex_jobs);
        if (sockfd == CLOSED) break;
        scheduler->sock->handle_request(sockfd);
    }

    return NULL;
}

void SCHEDULER::remove_job(int sockfd) {
    pthread_mutex_lock(&mutex_jobs);
    for (unsigned int i = 0; i < jobs.size(); i++) {
        if (jobs[i] == sockfd) {
            jobs.erase(jobs.begin() + i);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_jobs);
}

void SCHEDULER::terminate() {
    pthread_mutex_lock(&mutex_jobs);
    jobs.clear();
    for (int i = 0; i < NUM_THREADS; i++) {
        jobs.push_back(CLOSED);
    }
    pthread_cond_broadcast(&cond_jobs);
    pthread_mutex_unlock(&mutex_jobs);
}
