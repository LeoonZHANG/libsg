/**
 * log.c
 * Custom logging functions.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/sys/clock.h>
#include <sg/sys/shell.h>
#include <sg/str/vsstr.h>
#include <sg/sys/proc.h>

static bool log_colorful = true;
static FILE *fp_log;
static char fp_log_name[1024] = {0};

void sg_log_set_colorful(bool on)
{
    log_colorful = on;
}

void sg_logging(const char *file, int line, const char *func, enum sg_log_level lv, const char *fmt, ...)
{
    sg_vsstr_t *log;
    sg_vsstr_t *log_fmt;
    sg_vsstr_t *mod_path = sg_vsstr_alloc();
    va_list ap;
    char log_path[1024] = {0};
    char curr_date[100] = {0};
    char curr_time[100] = {0};

    if (!fmt) {
        printf("Logging param fmt error.\n");
        goto end;
    }

    va_start(ap, fmt);

    if (lv == SGLOGLEVEL_CRIT)
        log_fmt = sg_vsstr_fmt("file:%s, line:%d, function:%s\r\n%s", file, line, func, fmt);
    else
        log_fmt = sg_vsstr_fmt("%s\r\n", fmt);

    log = sg_vsstr_fmt_ap(sg_vsstr_raw(log_fmt), ap);

    if (sg_vsstr_len(log) == 0)
        goto end;

    printf("%s", sg_vsstr_raw(log));
    
    sg_proc_full_path(mod_path);
    sg_clock_curr_date_time("%Y-%m-%d", curr_date, 100);
    snprintf(log_path, 1024, "%s.%s.log", sg_vsstr_raw(mod_path), curr_date);
    if (strcmp(log_path, fp_log_name) != 0 && fp_log) {
        fclose(fp_log);
        fp_log = NULL;
    }
    snprintf(fp_log_name, 1024, "%s", log_path);
    
    if (!fp_log)
        fp_log = fopen(log_path, "ab+");
    if (fp_log) {
        sg_clock_curr_date_time("%Y-%m-%d %H:%M:%S ", curr_time, 100);
        fwrite(curr_time, 1, strlen(curr_time), fp_log);
        fwrite(sg_vsstr_raw(log), 1, sg_vsstr_len(log), fp_log);
        fflush(fp_log);
    }

end:
    va_end(ap);
    sg_vsstr_free(&log);
    sg_vsstr_free(&log_fmt);
}

/* have some bugs */
/*
void sg_logging(const char *file, int line, const char *func, enum sg_log_level lv, const char *fmt, ...)
{
    sg_vsstr_t *log;
    sg_vsstr_t *log_fmt;
    va_list ap;
    char mod_path[1024] = {0};
    char log_path[1024] = {0};
    char curr_date[100] = {0};
    char curr_time[100] = {0};

    if (!fmt) {
        printf("Logging param fmt error.\n");
        goto end;
    }

    va_start(ap, fmt);
    // Memory leaks.
    //log = sg_vsstr_alloc3(strlen(fmt) * 2 + 1);
    //if (!log) {
    //    printf("Logging open memory error.\n");
    //    goto end;
    //}

    //printf("heheda:%p.", ap);
    //printf("log param after fmt:%s", va_arg(ap, char *));

    if (lv == SGLOGLEVEL_CRIT)
        log_fmt = sg_vsstrfmt("file:%s, line:%d, function:%s\r\n%s", file, line, func, fmt);
    else
        log_fmt = sg_vsstrfmt("%s\r\n", fmt);

    log = sg_vsstr_fmt_ap(sg_vsstrraw(log_fmt), ap);
    //printf("log fmt:%s", sg_vsstrraw(log_fmt));
    //printf("log:%s", sg_vsstrraw(log));

    if (sg_vsstrlen(log) == 0)
        goto end;

    if (log_colorful == true) {
        switch (lv) {
        case SGLOGLEVEL_DBG:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_BLUE);
            break;
        case SGLOGLEVEL_INF:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_WHITE); // bug here!
            break;
        case SGLOGLEVEL_WARN:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_YELLOW);
            break;
        case SGLOGLEVEL_ERR:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_RED);
            break;
        case SGLOGLEVEL_CRIT:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_WHITE_ON_RED);
            printf("\n");
            break;
        case SGLOGLEVEL_ALERT:
            sg_shell_colorful_print(sg_vsstr_raw(log), SGSHELLCOLOR_WHITE_ON_RED);
            printf("\n");
            break;
        case SGLOGLEVEL_EMERG:
            sg_shell_colorful_print(sg_vsstrraw(log), SGSHELLCOLOR_WHITE_ON_RED);
            printf("\n");
            break;
        default:
            sg_shell_colorful_print("Log param level error.", SGSHELLCOLOR_RED);
            break;
        }
    } else
        printf("%s", sg_vsstrraw(log));
    
    sg_module_path(mod_path, 1024);
    sg_curr_date_time("%Y-%m-%d", curr_date, 100);
    snprintf(log_path, 1024, "%s.%s.log", mod_path, curr_date);
    if (strcmp(log_path, fp_log_name) != 0 && fp_log) {
        fclose(fp_log);
        fp_log = NULL;
    }
    snprintf(fp_log_name, 1024, "%s", log_path);
    
    if (!fp_log)
        fp_log = fopen(log_path, "ab+");
    if (fp_log) {
        sg_curr_date_time("%Y-%m-%d %H:%M:%S ", curr_time, 100);
        fwrite(curr_time, 1, strlen(curr_time), fp_log);
        fwrite(sg_vsstrraw(log), 1, sg_vsstr_len(log), fp_log);
        fflush(fp_log);
    }

end:
    va_end(ap);
    sg_vsstrfree(&log);
    sg_vsstr_free(&log_fmt);
}
*/

void sg_printf_time_stamp(const char *tag)
{
    sg_log_inf("%s:%lu", tag, sg_clock_boot_time_ms());
}
