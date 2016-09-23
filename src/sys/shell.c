/**
 * Shell.h
 * Bash and command shell IO and interactive APIs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sg/sg.h>

#if defined(SG_OS_WINDOWS)
# include <windows.h>
#endif
#include <sg/util/log.h>
#include <sg/sys/flag.h>
#include <sg/sys/shell.h>
#include <sg/sys/sleep.h>
#include <sg/util/trick.h>
#include <sg/sys/thread.h>

/* max line length in char */
#define line_size 1024

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
# define POPEN  popen
# define PCLOSE pclose
# define SHELL_COLORFUL_PRINT shell_colorful_print_psx
#elif defined(SG_OS_WINDOWS)
# define POPEN  _popen
# define PCLOSE _pclose
# define SHELL_COLORFUL_PRINT shell_colorful_print_win
#endif

/* standard out colors */
#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
# define IN_SHELLCOLOR_WHITE        "\033[0;37m"
# define IN_SHELLCOLOR_CYAN         "\033[0;36m"
# define IN_SHELLCOLOR_BLUE         "\033[0;34m"
# define IN_SHELLCOLOR_YELLOW       "\033[0;33m"
# define IN_SHELLCOLOR_RED          "\033[0;31m"
# define IN_SHELLCOLOR_WHITE_ON_RED "\033[0;37;41m"
# define IN_SHELLCOLOR_DEFAULT      "\033[0m"
#elif defined(SG_OS_WINDOWS)
# define IN_SHELLCOLOR_WHITE        ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )
# define IN_SHELLCOLOR_CYAN         ( FOREGROUND_GREEN | FOREGROUND_BLUE )
# define IN_SHELLCOLOR_BLUE         ( FOREGROUND_BLUE )
# define IN_SHELLCOLOR_YELLOW       ( FOREGROUND_RED | FOREGROUND_GREEN )
# define IN_SHELLCOLOR_RED          ( FOREGROUND_RED )
# define IN_SHELLCOLOR_WHITE_ON_RED ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED )
#endif

/* shell handle */
struct shell_data {
    FILE                *fp;        /* pipe file handle */
    char                *cmd;       /* command text */
    sg_flag_t           *run_flag;  /* thread run flag */
    void                *context;   /* user data */
    sg_thread_t         *thread;    /* thread */
    sg_shell_callback   cb;         /* shell callback function */
};

/* functions declaration */
void shell_work(void *p);
#if defined(SG_OS_WINDOWS)
void shell_colorful_print_win(const char *str, WORD color);
#endif
#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
void shell_colorful_print_psx(const char *str, const char *color);
#endif


int sg_shell_exec(const char *cmd, sg_shell_callback cb, void *context)
{
#define READ_BUF_SIZE 1024

    FILE *fp;
    char buf_read[READ_BUF_SIZE];

    SG_ASSERT(cmd);
    SG_ASSERT(strlen(cmd) > 0);
    SG_ASSERT(cb);

    errno = 0;
    fp = POPEN(cmd, "r"); /* Async, create pipe to sub process, and return stdout file. */
    if (!fp) {
        sg_log_err("Shell open failure, %s.", strerror(errno));
        return -1;
    }

    while (!feof(fp)) {
        if (fgets(buf_read, READ_BUF_SIZE, fp) == NULL)
            continue;
        cb(SGSHELLEVENT_DATA, buf_read, context);
        sg_sleep(1);
    }

    cb(SGSHELLEVENT_OVER, NULL, context);
    PCLOSE(fp);
    return 0;
}

int sg_shell_exec2(const char *cmd)
{
    FILE *fp;

    SG_ASSERT(cmd);
    SG_ASSERT(strlen(cmd) > 0);

    errno = 0;
    fp = POPEN(cmd, "w"); /* Sync, create pipe to sub process, and return stdin file. */
    if (!fp) {
        sg_log_err("Shell open failure, %s.", strerror(errno));
        return -1;
    }

    PCLOSE(fp);
    return 0;
}

sg_shell *sg_shell_open(const char *cmd, sg_shell_callback cb, void *context)
{
    struct shell_data *d;

    SG_ASSERT(cmd);
    SG_ASSERT(strlen(cmd) > 0);

    d = (struct shell_data *)malloc(sizeof(struct shell_data));
    if (!d) {
        sg_log_err("Shell data malloc failure.");
        return NULL;
    }

    d->cb       = cb;
    d->cmd      = strdup(cmd);
    d->context  = context;
    d->run_flag = sg_flag_create();

    if (!d->run_flag)
        sg_log_err("Shell flag create failure.");
    if (!d->cmd)
        sg_log_err("Shell command clone failure.");
    if (!d->run_flag || !d->cmd) {
        sg_flag_destroy(d->run_flag);
        if (d) {
            free(d);
            d = NULL;
        }
    }

    if (d) {
        sg_flag_write(d->run_flag, 1);
        d->thread = sg_thread_alloc(shell_work, (void *)d);
    }

    return d;
}

void sg_shell_close(sg_shell **s)
{
    SG_ASSERT(s);
    if (!*s)
        return;

    sg_flag_write((*s)->run_flag, 0);
    sg_thread_join((*s)->thread);
    if ((*s)->fp)
        PCLOSE((*s)->fp);
    sg_flag_destroy((*s)->run_flag);
    if ((*s)->cmd) {
        free((*s)->cmd);
        (*s)->cmd;
    }
    free(*s);
    *s = NULL;
}

/* Routine function for shell_open thread. */
void shell_work(void *p)
{
    char buf[line_size] = {0};
    struct shell_data *d = (struct shell_data *)p;

    if (!d)
        return;

    d->fp = POPEN(d->cmd, "r"); /* async */
    if (!d->fp) {
        sg_log_err("Shell open failure.");
        goto end;
    }

    while (sg_flag_read(d->run_flag) == 1 && fgets(buf, line_size, d->fp)) {
        if (d->cb)
            d->cb(SGSHELLEVENT_DATA, buf, d->context);
        sg_sleep(1);
    }

end:
    d->cb(SGSHELLEVENT_OVER, buf, d->context);
}

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
void shell_colorful_print_psx(const char *str, const char *color)
{
    sg_vsstr_t *s;

    s = sg_vsstr_fmt("%s%s%s", color, str, IN_SHELLCOLOR_DEFAULT);

    fprintf(stdout, "%s", sg_vsstr_raw(s));
    sg_vsstr_free(&s);
}
#elif defined(SG_OS_WINDOWS)
void shell_colorful_print_win(const char *str, WORD color)
{
    HANDLE h;
    WORD wOldColorAttrs;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

    h = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(h, &csbiInfo);
    wOldColorAttrs = csbiInfo.wAttributes;

    SetConsoleTextAttribute(h, color);
    fprintf(stdout, "%s", str);

    SetConsoleTextAttribute(h, wOldColorAttrs);
}
#endif

void sg_shell_colorful_print(const char *str, enum sg_shell_color color)
{
    SG_ASSERT(str);
    if (strlen(str) == 0)
        return;

    switch (color) {
    case SGSHELLCOLOR_WHITE:
        SHELL_COLORFUL_PRINT(str, IN_SHELLCOLOR_WHITE);
        break;
    case SGSHELLCOLOR_BLUE:
        SHELL_COLORFUL_PRINT(str, IN_SHELLCOLOR_BLUE);
        break;
    case SGSHELLCOLOR_YELLOW:
        SHELL_COLORFUL_PRINT(str, IN_SHELLCOLOR_YELLOW);
        break;
    case SGSHELLCOLOR_RED:
        SHELL_COLORFUL_PRINT(str, IN_SHELLCOLOR_RED);
        break;
    case SGSHELLCOLOR_WHITE_ON_RED:
        SHELL_COLORFUL_PRINT(str, IN_SHELLCOLOR_WHITE_ON_RED);
        break;
    default:
        SG_ASSERT(1 == 0);
        break;
    }
}
