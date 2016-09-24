/**
 * os.h
 * Operating system information detection.
 */

#ifndef LIBSG_OS_H
#define LIBSG_OS_H

#include <sg/sg.h>
#include <sg/str/vsstr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__MACH__) || defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
# define SG_OS_MACOS
#elif defined(linux) || defined(__linux) || defined(__linux__)
# define SG_OS_LINUX
#elif defined(__OpenBSD__)
# define SG_OS_OPENBSD
#elif defined(__ANDROID__)
# define SG_OS_ANDROID
#elif defined(IOS_VERSION)
# define SG_OS_IOS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
# define SG_OS_WINDOWS
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
# define SG_OS_WINDOWS
#else
# error Unsupported operating system.
#endif

/* unfinished */
#if defined(__x86_64__)
# define SG_OS_BITS_64
#else
# define OS_OS_BITS_32
#endif

/* Get distribution of OS. */
bool sg_os_distribution(sg_vsstr_t *str_out);

/* sg_start_with_os(const char *binary_filename);

/sg_not_start_with_os(const char *binary_filename); */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_OS_H */
