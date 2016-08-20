/*
 * uuid.c
 * Author: wangwei.
 * Universally unique ID library based on libuuid.
 */

#include <stdio.h>
#include <string.h>
#if defined(WIN32)
#include <Objbase.h>
typedef GUID uuid_t;

static void uuid_generate_win32(uuid_t* in)
{
	CoCreateGuid(&in);
}

static void uuid_unparse_lower_win32(uuid_t* id, char* out)
{
    int i, j;
    wchar_t ostr[32+6+3];
    StringFromGUID2(id, (LPOLESTR)ostr, _countof(ostr));
    for (i = 1, j = 0; i < 38 + 1; i++, j++)
        out[j] = tolower(ostr[i]);
}

# define uuid_generate(id)			uuid_generate_win32(&id)
# define uuid_generate_random       uuid_generate
# define uuid_generate_time         uuid_generate
# define uuid_generate_time_safe    uuid_generate
# define uuid_unparse_lower(id, out)	uuid_unparse_lower_win32(&id, out)

#else
#include <uuid/uuid.h>
#endif
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