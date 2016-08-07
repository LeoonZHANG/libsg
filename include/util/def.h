/*
 * macro.h
 * Author: wangwei.
 * Macros in common usage.
 */

#ifndef LIBSG_DEF_H
#define LIBSG_DEF_H

#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#ifndef MAX_URL
#define MAX_URL 2048
#endif

/* to delete */
#define SAFE_CLOSE_FILE(P)					if ((P) != NULL) { fclose(P); (P) = NULL; }
#define SAFE_CLOSE_HANDLE(P)				if ((P) != NULL) { CloseHandle(P); (P) = NULL; }
#define SAFE_FREE(P)						if ((P) != NULL) { free(P); (P) = NULL; }

#define ZERO(OBJ, TYPE) memset(&(OBJ), 0, sizeof(TYPE))

#ifndef MAX
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
#define MIN(A, B) ((A) > (B) ? (B) : (A))
#endif

/* By default, we use the standard "extern" declarations. */
#ifndef LIBSG_EXP_DECL
# ifdef __cplusplus
#  define LIBSG_EXP_DECL  extern "C"
# else
#  define LIBSG_EXP_DECL  extern
# endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DEF_H */
