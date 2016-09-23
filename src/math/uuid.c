/**
 * uuid.c
 * Universally unique ID library based on libuuid.
 */

#include <stdio.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/hash/uuid.h>

#if defined(SG_OS_WINDOWS)
# include <Objbase.h>
#endif

#ifndef SG_OS_WINDOWS
# include <uuid/uuid.h>
#endif

/* FIXME: There's no uuid_generate_time_safe on macOS, simply use regular one
   instead of *_safe. */
#if defined(SG_OS_MACOS)
# define uuid_generate_time_safe uuid_generate_time
#endif





#if defined(SG_OS_WINDOWS)

static void uuid_generate_win32(uuid_t* in)
{
    CoCreateGuid(in);
}

static void uuid_unparse_win32(uuid_t* id, char* out, int (__cdecl *func)(int))
{
    int i, j;
    wchar_t ostr[36+3];
    StringFromGUID2(id, (LPOLESTR)ostr, _countof(ostr));
    for (i = 1, j = 0; i < 36 + 1; i++, j++)
        out[j] = func(ostr[i]);
}

# define uuid_generate(id)          uuid_generate_win32(&id)
# define uuid_generate_random       uuid_generate
# define uuid_generate_time         uuid_generate
# define uuid_generate_time_safe    uuid_generate
# define uuid_unparse_lower(id, out)    uuid_unparse_win32(&id, out, tolower)
# define uuid_unparse_upper(id, out)    uuid_unparse_win32(&id, out, toupper)

#endif




void sg_uuid_gen(enum sg_uuid_method method, bool uppercase, struct sg_uuid_str *out_str)
{
    uuid_t uu;

    SG_ASSERT(method >= SGUUIDMETHOD_DEFAULT && method <= SGUUIDMETHOD_TIME_MAC);

    memset(out_str, 0, sizeof(struct sg_uuid_str));

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
        return;
    }

    if (uppercase)
        uuid_unparse_upper(uu, out_str->buf);
    else
        uuid_unparse_lower(uu, out_str->buf);
}
