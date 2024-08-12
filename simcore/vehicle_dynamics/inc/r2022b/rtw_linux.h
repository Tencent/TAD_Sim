#ifndef RTW_LINUX_H
#  define RTW_LINUX_H

/*
 * Copyright 2011-2022 The MathWorks, Inc.
 *
 * File: rtw_linux.h
 *
 * Abstract:
 *  Function prototypes and defines pthread mutex/semaphores
 */
#  include <pthread.h>
#  include <semaphore.h>
#  include <stdlib.h>

#  ifndef __USE_UNIX98
#    define __USE_UNIX98
#  endif

#  ifdef __cplusplus
extern "C" {
#  endif

extern void rtw_pthread_mutex_init(void **mutexDW);
extern void *rtw_register_task(void (*)(void));
extern void rtw_trigger_task(void *);
extern void rtw_waitfor_task(void *);
extern void rtw_deregister_task(void *);

#  ifdef __cplusplus
}
#  endif

#  define rtw_pthread_mutex_lock(mutexDW) pthread_mutex_lock((pthread_mutex_t *)(mutexDW));

#  define rtw_pthread_mutex_unlock(mutexDW) pthread_mutex_unlock((pthread_mutex_t *)(mutexDW));

#  define rtw_pthread_mutex_destroy(mutexDW)             \
    pthread_mutex_destroy((pthread_mutex_t *)(mutexDW)); \
    free(mutexDW);

#  define rtw_pthread_sem_create(semaphoreDW, initVal) \
    *semaphoreDW = malloc(sizeof(sem_t));              \
    sem_init((sem_t *)(*semaphoreDW), 0, (initVal));

#  define rtw_pthread_sem_wait(semaphoreDW) sem_wait((sem_t *)(semaphoreDW));

#  define rtw_pthread_sem_post(semaphoreDW) sem_post((sem_t *)(semaphoreDW));

#  define rtw_pthread_sem_destroy(semaphoreDW) \
    sem_destroy((sem_t *)semaphoreDW);         \
    free(semaphoreDW);

#endif /* RTW_LINUX_H */

/* LocalWords:  pthread
 */
