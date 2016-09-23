/**
 * cron.c
 * Functions for cron job.
 */

#include <stdint.h>
#include <string.h> /* memset */
#include <sg/sg.h>
#include <sg/sys/cron.h>
#include <sg/sys/clock.h>

int sg_cron_cycle_pack(struct tm *begin, struct tm *end,
                       uint32_t duration_s, uint32_t interval_s, struct sg_cron_cycle *cc)
{
    int ret = 0;

    SG_ASSERT(cc);
    if (!begin && !end) {
        sg_log_err("Both begin time and end time are invalid.");
        return -1;
    }
    if (duration_s == 0 && interval_s != 0) {
        sg_log_err("Duration is 0, but interval is not.");
        return -1;
    }
    if (duration_s != 0 && interval_s == 0) {
        sg_log_err("Duration is not 0, but interval is.");
        return -1;
    }
    if (begin && end && sg_clock_date_time_s_diff(*end, *begin) < 0) {
        sg_log_err("Begin time is bigger than end time.");
        return -1;
    }

    cc->use_begin   = begin ? 1 : 0;
    cc->use_end     = end ? 1 : 0;
    cc->duration_ms = duration_s * 1000;
    cc->interval_ms = interval_s * 1000;
    memset(&cc->begin, 0, sizeof(struct timespec));
    memset(&cc->end, 0, sizeof(struct timespec));
    if (begin)
        ret += sg_clock_date_time_s_to_unix_time_ns(begin, &(cc->begin));
    if (end)
        ret += sg_clock_date_time_s_to_unix_time_ns(end, &(cc->end));
    if (ret != 0) {
        sg_log_err("Time make error.");
        return -1;
    }

    return 0;
}

int sg_cron_cycle_check(struct sg_cron_cycle *cc,
                        struct timespec check_time, int32_t *cycle_passed_ms, int32_t *cycle_left_ms)
{
    /* a deviation value from a reference value 'cc->begin' */
    long chk_ref_begin_ms;
    /* a deviation value from a reference value 'cycle begin time' */
    long chk_ref_cycle_ms;
    /* cycle duration value in milliseconds */
    long cycle_ms;

    SG_ASSERT(cc);
    if (cc->duration_ms == 0 && cc->interval_ms != 0)
        return -1;
    if (cc->duration_ms != 0 && cc->interval_ms == 0)
        return -1;
    if (!cc->use_begin && !cc->use_end)
        return -1;
    if (cc->use_begin && check_time.tv_sec < cc->begin.tv_sec)
        return 0;
    if (cc->use_end && check_time.tv_sec > cc->end.tv_sec)
        return 0;
    /*
    unix_time_ns_to_str(check_time, swap, 120);
    unix_time_ns_to_str(cc->end, swap, 120);
    sg_log(MLL_INFO, "cc check end time :%s", swap);
    */

    /* Set 'cc->begin' as origin. */
    chk_ref_begin_ms = ((check_time.tv_sec - cc->begin.tv_sec) * 1000) +
            (sg_clock_ns_to_ms(check_time.tv_nsec) - sg_clock_ns_to_ms(cc->begin.tv_nsec));

    /* Duration or interval equals 0 means all the time. */
    if (cc->duration_ms == 0 || cc->interval_ms == 0) {
        if (cycle_passed_ms)
            *cycle_passed_ms = (uint32_t)chk_ref_begin_ms;
        if (cycle_left_ms)
            *cycle_left_ms = -1; /* invalid */
        return 1;
    }

    /* Calculate regular cycle. */
    cycle_ms = cc->duration_ms + cc->interval_ms;
    chk_ref_cycle_ms = chk_ref_begin_ms % cycle_ms;
    if (chk_ref_cycle_ms <= cc->duration_ms) {
        if (cycle_passed_ms)
            *cycle_passed_ms = (uint32_t)chk_ref_cycle_ms;
        if (cycle_left_ms)
            *cycle_left_ms = cc->duration_ms - (uint32_t)chk_ref_cycle_ms;
        return 1;
    } else
        return 0;
}

int sg_cron_cycle_check_now(struct sg_cron_cycle *cc,
                            int32_t *cycle_passed_ms, int32_t *cycle_left_ms)
{
    return sg_cron_cycle_check(cc, sg_clock_unix_time_ns(), cycle_passed_ms, cycle_left_ms);
}
