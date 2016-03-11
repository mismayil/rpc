#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <deque>
#include <vector>
#include <pthread.h>

#define NUM_THREADS    1

class SOCK;

typedef int (*FUNC)(SOCK*, int);

class SCHEDULER {
    SOCK *sock;
    FUNC func;
    pthread_t threads[NUM_THREADS];
    std::vector<int> jobs;
    pthread_cond_t cond_jobs;
    pthread_mutex_t mutex_jobs;
    pthread_cond_t cond_bargers;
    bool signalled;
public:
    SCHEDULER(SOCK *sock, FUNC func);
    ~SCHEDULER();
    int run();
    void add_job(int i);
    static void *execute_job(void *ptr);
    void remove_job(int i);
    void terminate();
};

#endif
