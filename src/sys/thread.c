/*
 * thread.c
 * Author: wangwei.
 * Platform independent implementation of threading.
 */

#include <signal.h>
#include <sg/sys/thread.h>
#include <sg/util/assert.h>

#if defined(OS_WIN)
static unsigned int __stdcall thread_main_routine(void *arg)
{
    struct sg_thread *self;

    self = (struct sg_thread *)arg;
    self->routine(self->arg); /* Run the thread routine. */

    return 0;
}
#else
static void *thread_main_routine(void *arg)
{
    struct sg_thread *self;

    self = (struct sg_thread *)arg;
    self->routine(self->arg); /* Run the thread routine. */

    return NULL;
}
#endif

#if defined(OS_WIN)
void sg_thread_init(struct sg_thread *self, sg_thread_routine *routine, void *arg)
{
    self->routine = routine;
    self->arg = arg;
    self->handle = (HANDLE)_beginthreadex(NULL, 0, thread_main_routine,
                                          (void *)self, 0 , NULL);
    assert(self->handler != NULL);
}
#else
void sg_thread_init(struct sg_thread *self, sg_thread_routine *routine, void *arg)
{
    int res;
    sigset_t new_sigmask;
    sigset_t old_sigmask;

    /* No signals should be processed by this thread. The library doesn't
       use signals and thus all the signals should be delivered to application
       threads, not to worker threads. */
    res = sigfillset(&new_sigmask);
    assert(res == 0);
    res = pthread_sigmask(SIG_BLOCK, &new_sigmask, &old_sigmask);
    assert(res == 0);

    self->routine = routine;
    self->arg = arg;
    res = pthread_create(&self->handle, NULL, thread_main_routine, (void *)self);
    assert(res == 0);

    /* Restore signal set to what it was before. */
    res = pthread_sigmask(SIG_SETMASK, &old_sigmask, NULL);
    assert(res == 0);
}
#endif

#if defined(OS_WIN)
void thread_join(struct sg_thread *self)
{
    DWORD res;
    BOOL bres;

    res = WaitForSingleObject(self->handle, INFINITE);
    assert(res != WAIT_FAILED);
    bres = CloseHandle(self->handle);
    assert(bres != 0);
}
#else
void sg_thread_join(struct sg_thread *self)
{
    int res;

    res = pthread_join(self->handle, NULL);
    assert(res == 0);
}
#endif