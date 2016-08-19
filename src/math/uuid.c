/*
 * uuid.c
 * Author: wangwei.
 * Universally unique ID library based on libuuid.
 */

#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>
/* FIXME: There's no uuid_generate_time_safe on macOS, simply use regular one
   instead of *_safe. */
#if defined(__MACH__)
# define uuid_generate_time_safe uuid_generate_time
#endif
#include "../../include/sg/util/log.h"
#include "../../include/sg/math/uuid.h"
#include "../../include/sg/util/assert.h"

struct sg_uuid_str sg_uuid_gen(enum sg_uuid_method method, int uppercase)
{
    uuid_t uu;
    struct sg_uuid_str us;

    assert(method >= SGUUIDMETHOD_DEFAULT && method <= SGUUIDMETHOD_TIME_MAC);

    memset(&us, 0, sizeof(struct sg_uuid_str));

    switch (method) {
    case SGUUIDMETHOD_DEFAULT:
        uuid_generate(uu);
        break;
    case SGUUIDMETHOD_RANDOM:
        uuid_generate_random(uu);
        break;
    case SGUUIDMETHOD_TIME_MAC:
        uuid_generate_time(uu);
        break;
    case SGUUIDMETHOD_TIME_MAC_SAFE:
        uuid_generate_time_safe(uu);
        break;
    default:
        sg_log_err("UUID method error.");
        return us;
    }

    if (uppercase == 0)
        uuid_unparse_lower(uu, us.buf);
    else
        uuid_unparse_upper(uu, us.buf);

    return us;
}