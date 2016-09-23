/**
 * rwlock.c
 * Test for util module rwlock.
 */

#include "../util/log.h"
#include "../util/time.h"
#include "../util/sleep.h"
#include "../util/assert.h"
#include "../util/rwlock.h"
#include "../util/thread.h"

static int rw_val = 0;
static const int cycle = 10000;
static unsigned long nolock_cost_ms = 0;
static unsigned long rwlock_cost_ms = 0;

void nolock_read_task(void *p);
void nolock_write_task_a(void *p);
void nolock_write_task_b(void *p);
void rwlock_read_task(void *p);
void rwlock_write_task_a(void *p);
void rwlock_write_task_b(void *p);

void nolock_read_task(void *p)
{
    int res = rw_val;
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        if(rw_val < res) {
            sg_log_err("Read error.");
            assert(0);
        }
        res = rw_val;
        sleep_ms(1);
    }
    nolock_cost_ms += thread_cpu_time_ms() - start;
}

void nolock_write_task_a(void *p)
{
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        rw_val ++;
        sleep_ms(1);
    }
    nolock_cost_ms += thread_cpu_time_ms() - start;
}

void nolock_write_task_b(void *p)
{
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        rw_val ++;
        sleep_ms(1);
    }
    nolock_cost_ms += thread_cpu_time_ms() - start;
}

void rwlock_read_task(void *p)
{
    int res;
    rwlock *lock = (struct rwlock *)p;
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        rwlock_r_lock(lock);
        res = rw_val;
        rwlock_r_unlock(lock);
        sleep_ms(1);
    }
    rwlock_cost_ms += thread_cpu_time_ms() - start;
}

void rwlock_write_task_a(void *p)
{
    rwlock *lock = (struct rwlock *)p;
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        rwlock_w_lock(lock);
        rw_val++;
        rwlock_w_unlock(lock);
        sleep_ms(1);
    }
    rwlock_cost_ms += thread_cpu_time_ms() - start;
}

void rwlock_write_task_b(void *p)
{
    rwlock *lock = (struct rwlock *)p;
    unsigned long start;

    start = thread_cpu_time_ms();
    for(int i = 0; i < cycle; i++) {
        rwlock_w_lock(lock);
        rw_val++;
        rwlock_w_unlock(lock);
        sleep_ms(1);
    }
    rwlock_cost_ms += thread_cpu_time_ms() - start;
}

int main (void)
{
    rwlock *lock;
    int idx;
    struct mw_thread thd_nolock_read;
    struct mw_thread thd_nolock_write_a;
    struct mw_thread thd_nolock_write_b;
    struct mw_thread thd_rwlock_read;
    struct mw_thread thd_rwlock_write_a;
    struct mw_thread thd_rwlock_write_b;

    lock = rwlock_create();

    thread_init(&(thd_nolock_read), nolock_read_task, NULL);
    thread_init(&(thd_nolock_write_a), nolock_write_task_a, NULL);
    thread_init(&(thd_nolock_write_b), nolock_write_task_b, NULL);
    thread_join(&thd_nolock_read);
    thread_join(&thd_nolock_write_a);
    thread_join(&thd_nolock_write_b);
    sg_log_inf("Multi thread read write without lock result: %d.", rw_val);

    rw_val = 0;
    thread_init(&(thd_rwlock_read), rwlock_read_task, (void *)(lock));
    thread_init(&(thd_rwlock_write_a), rwlock_write_task_a, (void *)(lock));
    thread_init(&(thd_rwlock_write_b), rwlock_write_task_b, (void *)(lock));
    thread_join(&thd_rwlock_read);
    thread_join(&thd_rwlock_write_a);
    thread_join(&thd_rwlock_write_b);
    sg_log_inf("Multi thread read write with rwlock result: %d.", rw_val);

    if(rw_val == cycle * 2)
        sg_log_dbg("rwlock module test passed.");
    else
        sg_log_err("rwlock module test failed to pass.");

    return 0;
}
