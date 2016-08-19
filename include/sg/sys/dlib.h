/*
 * dlib.h
 * Author: wangwei.
 * Dynamic library handle.
 */

#ifndef LIBSG_DLIB_H
#define LIBSG_DLIB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* handle type */
typedef void sg_dlib;

/* Open dynamic library handle. */
sg_dlib *sg_dlib_open(const char *path);

/* Load function address by symbol from dynamic library. */
void *sg_dlib_symbol(sg_dlib *handle, const char *symbol);

/* Close dynamic library handle. */
void sg_dlib_close(sg_dlib **handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DLIB_H */