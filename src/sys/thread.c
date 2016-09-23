/*
 * thread.c
 * Author: wangwei.
 * Platform independent implementation of threading.
 */

#include <stdlib.h>
#include <signal.h>
#include <sg/sg.h>
#include <sg/sys/thread.h>

#if defined(SG_OS_WINDOWS)
# include <Windows.h>
# include <process.h>
#else
# include <pthread.h>
#endif

struct sg_thread_real {
    void                *arg;
#if defined(SG_OS_WINDOWS)
    HANDLE              handle;
#else
    pthread_t           handle;
#endif
    sg_thread_routine_func_t   *routine;
};

#if defined(SG_OS_WINDOWS)
static unsigned int __stdcall thread_main_routine(void *arg)
{
    struct sg_thread_real *self;

    self = (struct sg_thread *)arg;
    self->routine(self->arg); /* Run the thread routine. */

    return 0;
}
#else
static void *thread_main_routine(void *arg)
{
    struct sg_thread_real *self;

    self = (struct sg_thread_real *)arg;
    self->routine(self->arg); /* Run the thread routine. */

    return NULL;
}
#endif


sg_thread_t *sg_thread_alloc(sg_thread_routine_func_t *routine, void *arg)
{
#if defined(SG_OS_WINDOWS)

    struct sg_thread_real *self = NULL;

    self = (struct sg_thread_real *)malloc(sizeof(struct sg_thread_real));
    if (!self)
        return NULL;

    self->routine = routine;
    self->arg = arg;
    self->handle = (HANDLE)_beginthreadex(NULL, 0, thread_main_routine,
                                          (void *)self, 0 , NULL);
    assert(self->handle != NULL);
    return (sg_thread_t *)self;

#else

    int res;
    sigset_t new_sigmask;
    sigset_t old_sigmask;
    struct sg_thread_real *self;

    self = (struct sg_thread_real *) malloc(sizeof(struct sg_thread_real));
    if (!self)
        return NULL;

    /* No signals should be processed by this thread. The library doesn't
       use signals and thus all the signals should be delivered to application
       threads, not to worker threads. */
    res = sigfillset(&new_sigmask);
    assert(res == 0);
    res = pthread_sigmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    assert(res == 0);

    self->routine = routine;
    self->arg = arg;
    res = pthread_create(&self->handle, NULL, thread_main_routine, (void *) self);
    assert(res == 0);

    /* Restore signal set to what it was before. */
    res = pthread_sigmask(SIG_SETMASK, &old_sigmask, NULL);
    assert(res == 0);

#endif
}

void sg_thread_join(sg_thread_t *self)
{
#if defined(SG_OS_WINDOWS)

    DWORD res;
    BOOL bres;

    res = WaitForSingleObject(((struct sg_thread_real *)self)->handle, INFINITE);
    assert(res != WAIT_FAILED);
    bres = CloseHandle(((struct sg_thread_real *)self)->handle);
    assert(bres != 0);

#else

    int res;

    res = pthread_join(((struct sg_thread_real *)self)->handle, NULL);
    assert(res == 0);

#endif
}

void sg_thread_free(sg_thread_t *self)
{
    if (self)
        free(self);
}