/**
 * uuid.h
 * Universally unique ID library based on libuuid.
 */

#ifndef LIBSG_UUID_H
#define LIBSG_UUID_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  generate way */
enum sg_uuid_method {
    /* The uuid will be generated based on high-quality randomness
       from /dev/urandom, if available. If it is not available,
       then uuid_generate will use an alternative algorithm which
       uses the current time, the local ethernet MAC address (if available),
       and random data generated using a pseudo-random generator. */
    SGUUIDMETHOD_DEFAULT  = 0,
    /* Forces the use of the all-random UUID format, even if a
       high-quality random number generator (i.e., /dev/urandom)
       is not available, in which case a pseudo-random generator
       will be subsituted. Note that the use of a pseudo-random
       generator may compromise the uniqueness of UUID's generated
       in this fashion. */
    SGUUIDMETHOD_RANDOM   = 1,
    /* Forces the use of the alternative algorithm which uses the
       current time and the local ethernet MAC address (if available).
       This algorithm used to be the default one used to generate UUID,
       but because of the use of the ethernet MAC address, it can leak
       information about when and where the UUID was generated.
       This can cause privacy problems in some applications, so the
       uuid_generate function only uses this algorithm if a high-quality
       source of randomness is not available. */
    SGUUIDMETHOD_TIME_MAC = 2,
    SGUUIDMETHOD_TIME_MAC_SAFE = 3,
};

/* uuid 36 characters string */
struct sg_uuid_str {
    char buf[37];
};

/* Creates a new UUID. */
void sg_uuid_gen(enum sg_uuid_method method, bool uppercase, struct sg_uuid_str *out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_UUID_H */
