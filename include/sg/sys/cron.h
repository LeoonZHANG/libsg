/**
 * cron.h
 * Functions for cron job.
 */

#ifndef LIBSG_CRON_H
#define LIBSG_CRON_H

#include <sg/sg.h>
#include <time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************
 * Cycle Cron:
 *
 * begin time                                                       end time
 *      |                                                              |
 *      +----------cycle----------+----------cycle----------+---cycle--+
 *      |---duration---|-interval-|---duration---|-interval-|-duration-|
 *
 *****************************************************************************/

struct sg_cron_cycle {
    struct timespec begin;       /* Begin date time, if it is zero means start right now. */
    struct timespec end;         /* End date time, if it is zero means endless. */
    int             use_begin;   /* Limit begin time or not. */
    int             use_end;     /* Limit end time or not. */
    uint32_t        duration_ms; /* Cron duration in one cycle, 0 means ignore interval. */
    uint32_t        interval_ms; /* Milliseconds between two duration, 0 means ignore interval. */
};

/*
 * Make a CronCycle value.
 */
int sg_cron_cycle_pack(struct tm *begin, struct tm *end,
                       uint32_t duration_s, uint32_t interval_s, struct sg_cron_cycle *cc);

/*
 * Check whether 'check_time' is in the cron 'cc'.
 * If it is in, output passed and left milliseconds in cycle.
 * If cycle_passed_ms/cycle_left_ms is negative, means invalid,
 * such as in a endless cron, cycle_left_ms is invalid.
 */
int sg_cron_cycle_check(struct sg_cron_cycle *cc,
                        struct timespec check_time, int32_t *cycle_passed_ms, int32_t *cycle_left_ms);

/*
 * Check whether current time is in the cron 'cc'.
 * If it is in, output passed and left milliseconds in cycle.
 */
int sg_cron_cycle_check_now(struct sg_cron_cycle *cc,
                            int32_t *cycle_passed_ms, int32_t *cycle_left_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CRON_H */