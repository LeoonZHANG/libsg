/*
 * c_float.h
 * Author: wangwei.
 * Extended API for regular integer, float and double number in C.
 */

/*http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
http://blog.chinaunix.net/uid-20279362-id-3848329.html*/

#ifndef LIBSG_C_INT_H
#define LIBSG_C_INT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * TODO:
 * Add overflow check.
 */
 
enum sg_c_int_type {
    SGCINTTYPE_SCHAR,   /* char */
    SGCINTTYPE_UCHAR,   /* unsigned char */
    SGCINTTYPE_SSHORT,  /* short */
    SGCINTTYPE_USHORT,  /* unsigned short */
    SGCINTTYPE_SINT,    /* int */
    SGCINTTYPE_UINT,    /* unsigned int */
    SGCINTTYPE_SINT32,  /* int32_t */
    SGCINTTYPE_UINT32,  /* uint32_t */
    SGCINTTYPE_SINT64,  /* int64_t, long long(some platform long long equals int64_t , some not) */
    SGCINTTYPE_UINT64,  /* uint64_t */
    SGCINTTYPE_SLONG,   /* long, long int */
    SGCINTTYPE_ULONG    /* unsigned long, unsigned long int */
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_C_INT_H */
