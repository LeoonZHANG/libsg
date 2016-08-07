/*
 * os.h
 * Author: wangwei.
 * Operating system information detection.
 */

#ifndef LIBSG_OS_H
#define LIBSG_OS_H

#include "../str/vlstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 摘自最新的 Win7 SDK 中的 sdkddkver.h */
/*#define _WIN32_WINNT_NT4                               0x0400
#define _WIN32_WINNT_WIN2K                           0x0500
#define _WIN32_WINNT_WINXP               0x0501
#define _WIN32_WINNT_WS03                  0x0502
#define _WIN32_WINNT_WIN6                 0x0600
#define _WIN32_WINNT_VISTA                0x0600
#define _WIN32_WINNT_WS08                 0x0600
#define _WIN32_WINNT_LONGHORN           0x0600
#define _WIN32_WINNT_WIN7                  0x0601

enum os_win_ver {
    OSWINVER_2000 = 0,
    OSWINVER_XP = 1,
    OSWINVER_VISTA = 2,
    OSWINVER_7 = 2,
    OSWINVER_8 = 2,
    OSWINVER_10 = 2,
};*/

#if defined(__MACOSX__)
# define OS_OSX
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define OS_LNX
#elif defined(__OpenBSD__)
# define OS_OPENBSD
#elif defined(__ANDROID__)
# define OS_ANDROID
#elif defined(IOS_VERSION)
# define OS_IOS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
# define OS_WIN
# define OS_32
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
# define OS_WIN
# define OS_64
#else
# error Unsupported operating system.
#endif

#if defined(__x86_64__)
# define OS_64
#else
# define OS_32
#endif

/* Get distribution of OS. */
sg_vlstr *sg_os_distribution(void);

/* sg_start_with_os(const char *binary_filename);

/sg_not_start_with_os(const char *binary_filename); */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_OS_H */
