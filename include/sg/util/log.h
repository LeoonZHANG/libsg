/**
 * log.h
 * Custom logging functions.
 */

#ifndef LIBSG_LOG_H
#define LIBSG_LOG_H

#include "../sg.h"

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

void sg_log_set_colorful(bool on);

/* logging */
void sg_logging(const char *file, int line, const char *func, enum sg_log_level lv, const char *fmt, ...);

/* format and print time stamp with tag followed */
void sg_printf_time_stamp(const char *tag);


#define sg_log(lv, fmt, ...)\
    sg_logging(__FILE__, __LINE__, __FUNCTION__, lv, fmt, ##__VA_ARGS__)


#define sg_log_dbg(fmt, ...)    sg_log(SGLOGLEVEL_DBG, fmt, ##__VA_ARGS__)

#define sg_log_notice(fmt, ...) sg_log(SGLOGLEVEL_NOTICE, fmt, ##__VA_ARGS__)

#define sg_log_inf(fmt, ...)    sg_log(SGLOGLEVEL_INF, fmt, ##__VA_ARGS__)

#define sg_log_warn(fmt, ...)   sg_log(SGLOGLEVEL_WARN, fmt, ##__VA_ARGS__)

#define sg_log_err(fmt, ...)    sg_log(SGLOGLEVEL_ERR, fmt, ##__VA_ARGS__)

#define sg_log_crit(fmt, ...)   sg_log(SGLOGLEVEL_CRIT, fmt, ##__VA_ARGS__)

#define sg_log_alert(fmt, ...)  sg_log(SGLOGLEVEL_ALERT, fmt, ##__VA_ARGS__)

#define sg_log_emerg(fmt, ...)  sg_log(SGLOGLEVEL_EMERG, fmt, ##__VA_ARGS__)

#define sg_log_errno(lv, errno) sg_log(lv, sg_err_fmt(errno))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_LOG_H */