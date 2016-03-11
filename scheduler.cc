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

SCHEDULER::SCHEDULER(SOCK *sock, FUNC func): sock(sock), func(func), signalled(false) {
    pthread_mutex_init(&mutex_jobs, NULL);
    pthread_cond_init(&cond_jobs, NULL);
    pthread_cond_init(&cond_bargers, NULL);
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

void SCHEDULER::add_job(int i) {
    pthread_mutex_lock(&mutex_jobs);
    DEBUG("ADD JOB", i);
    if (signalled && jobs.size() > 0) pthread_cond_wait(&cond_bargers, &mutex_jobs);
    for (unsigned int k = 0; k < jobs.size(); k++) {
        if (jobs[k] == i) {
            pthread_mutex_unlock(&mutex_jobs);
            return;
        }
    }
    jobs.push_back(i);
    signalled = true;
    pthread_cond_signal(&cond_jobs);
    pthread_mutex_unlock(&mutex_jobs);
}

void* SCHEDULER::execute_job(void *ptr) {
    SCHEDULER *scheduler = (SCHEDULER *) ptr;

    while (1) {
        pthread_mutex_lock(&scheduler->mutex_jobs);
        INFO("waiting for a job...");
        if (!scheduler->signalled && scheduler->jobs.size() == 0) pthread_cond_wait(&scheduler->cond_jobs, &scheduler->mutex_jobs);
        int i = scheduler->jobs.back();
        DEBUG("EXEC JOB", i);
        scheduler->jobs.pop_back();
        pthread_cond_signal(&scheduler->cond_bargers);
        scheduler->signalled = false;
        pthread_mutex_unlock(&scheduler->mutex_jobs);
        scheduler->func(scheduler->sock, i);
    }

    return NULL;
}

void SCHEDULER::remove_job(int i) {
    pthread_mutex_lock(&mutex_jobs);
    jobs.erase(jobs.begin() + i);
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
