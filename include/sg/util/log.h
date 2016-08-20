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
#define sg_log(lv, fmt, ...)\
av_log(NULL,\
       lv == SGLOGLEVEL_ERR || lv == SGLOGLEVEL_CRIT ? AV_LOG_ERROR : AV_LOG_INFO,\
       fmt"\n", ##__VA_ARGS__)
#else
/* custom logging */
#define sg_log(lv, fmt, ...)\
    sg_logging(__FILE__, __LINE__, __FUNCTION__, lv, fmt, ##__VA_ARGS__)
#endif


/* logging debug level message */
#define sg_log_dbg(fmt, ...)    sg_log(SGLOGLEVEL_DBG, fmt, ##__VA_ARGS__)

/* logging notice level message */
#define sg_log_notice(fmt, ...) sg_log(SGLOGLEVEL_NOTICE, fmt, ##__VA_ARGS__)

/* logging information level message */
#define sg_log_inf(fmt, ...)    sg_log(SGLOGLEVEL_INF, fmt, ##__VA_ARGS__)

/* logging warning level message */
#define sg_log_warn(fmt, ...)   sg_log(SGLOGLEVEL_WARN, fmt, ##__VA_ARGS__)

/* logging error level message */
#define sg_log_err(fmt, ...)    sg_log(SGLOGLEVEL_ERR, fmt, ##__VA_ARGS__)

/* logging critical level message */
#define sg_log_crit(fmt, ...)   sg_log(SGLOGLEVEL_CRIT, fmt, ##__VA_ARGS__)

/* logging alert level message */
#define sg_log_alert(fmt, ...)  sg_log(SGLOGLEVEL_ALERT, fmt, ##__VA_ARGS__)

/* logging emerge level message */
#define sg_log_emerg(fmt, ...)  sg_log(SGLOGLEVEL_EMERG, fmt, ##__VA_ARGS__)

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