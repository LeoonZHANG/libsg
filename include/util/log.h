/*
 * log.h
 * Author: wangwei.
 * Custom logging functions.
 */

#ifndef LIBSG_LOG_H
#define LIBSG_LOG_H

/* #define USE_AV_LOG */

#ifdef USE_AV_LOG
# include "../../libavutil/log.h"
#endif
#include "../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* log level */
enum sg_log_level {
    SGLOGLEVEL_DBG      = 0,
    SGLOGLEVEL_INF      = 1,
    SGLOGLEVEL_NOTICE   = 2,
    SGLOGLEVEL_WARN     = 3,
    SGLOGLEVEL_ERR      = 4,
    SGLOGLEVEL_CRIT     = 5,
    SGLOGLEVEL_ALERT    = 6,
    SGLOGLEVEL_EMERG    = 7
};

void sg_log_set_colorful(void);

void sg_log_cancel_colorful(void);

/* logging */
void sg_logging(const char *file, int line, const char *func, enum sg_log_level lv, const char *fmt, ...);

/* format and print time stamp with tag followed */
void sg_printf_time_stamp(const char *tag);

#ifdef USE_AV_LOG
/* Use av_log. */
#define sg_log(lv, fmt, args...)\
av_log(NULL,\
       lv == SGLOGLEVEL_ERR || lv == SGLOGLEVEL_CRIT ? AV_LOG_ERROR : AV_LOG_INFO,\
       fmt"\n", ##args)
#else
/* custom logging */
#define sg_log(lv, fmt, args...)\
    sg_logging(__FILE__, __LINE__, __FUNCTION__, lv, fmt, ##args)
#endif


/* logging debug level message */
#define sg_log_dbg(fmt, args...)    sg_log(SGLOGLEVEL_DBG, fmt, ##args)

/* logging notice level message */
#define sg_log_notice(fmt, args...) sg_log(SGLOGLEVEL_NOTICE, fmt, ##args)

/* logging information level message */
#define sg_log_inf(fmt, args...)    sg_log(SGLOGLEVEL_INF, fmt, ##args)

/* logging warning level message */
#define sg_log_warn(fmt, args...)   sg_log(SGLOGLEVEL_WARN, fmt, ##args)

/* logging error level message */
#define sg_log_err(fmt, args...)    sg_log(SGLOGLEVEL_ERR, fmt, ##args)

/* logging critical level message */
#define sg_log_crit(fmt, args...)   sg_log(SGLOGLEVEL_CRIT, fmt, ##args)

/* logging alert level message */
#define sg_log_alert(fmt, args...)  sg_log(SGLOGLEVEL_ALERT, fmt, ##args)

/* logging emerge level message */
#define sg_log_emerg(fmt, args...)  sg_log(SGLOGLEVEL_EMERG, fmt, ##args)

#define sg_log_check_alloc(ptr) \
    if (!(ptr)) \
        sg_log_crit("Out of memory when %s allocated.", #ptr);

//?
#define sg_log_errno() \
    sg_log()

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_LOG_H */