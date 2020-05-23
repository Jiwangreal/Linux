//
// Created by wangji on 19-8-15.
//

// p41 线程池

#ifndef NETWORKPROGRAMMING_CONDITION_H
#define NETWORKPROGRAMMING_CONDITION_H

#include <pthread.h>

typedef struct condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;//条件变量总是和互斥锁一起使用
} condition_t;

int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);//对互斥锁进行锁定
int condition_unlock(condition_t *cond);//对互斥锁进行解锁
int condition_wait(condition_t *cond);//等待条件
int condition_timewait(condition_t *cond, const struct timespec *abstime);//超时登台
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);


#endif //NETWORKPROGRAMMING_CONDITION_H
