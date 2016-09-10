cmake_minimum_required(VERSION 2.8)


# project
#########
project(libc99-snprintf)

set(lib_name c99-snprintf)
set(lib_name_static ${lib_name})
## Take different names for static lib and dynamic lib on Windows, or they're .lib file will conflicted.
if(WIN32)
    set(lib_name_static lib${lib_name_static})
endif()
set(target_static lib${lib_name}-static)


# files
#######
file(GLOB_RECURSE src_lib snprintf.c)


# build options
###############
include_directories(include)
add_definitions(
    -DHAVE_VSNPRINTF=0
    -DHAVE_SNPRINTF=0
    -DHAVE_VASPRINTF=0
    -DHAVE_ASPRINTF=0
    -DHAVE_STDARG_H=1
    -DHAVE_STDDEF_H=1
    -DHAVE_STDINT_H=1
    -DHAVE_STDLIB_H=1
    -DHAVE_INTTYPES_H=0
    -DHAVE_LOCALE_H=1
    -DHAVE_LOCALECONV=1
    -DHAVE_LCONV_DECIMAL_POINT=1
    -DHAVE_LCONV_THOUSANDS_SEP=1
    -DHAVE_LONG_DOUBLE=1
    -DHAVE_LONG_LONG_INT=1
    -DHAVE_UNSIGNED_LONG_LONG_INT=1
    -DHAVE_INTMAX_T=1
    -DHAVE_UINTMAX_T=1
    -DHAVE_UINTPTR_T=1
    -DHAVE_PTRDIFF_T=1
    -DHAVE_VA_COPY=1
    -DHAVE___VA_COPY=1
    -Dvasprintf=rpl_vasprintf
    -Dasprintf=rpl_asprintf
    )


# targets
#########
add_library(${target_static} STATIC ${src_lib})

set_target_properties(${target_static} PROPERTIES
    OUTPUT_NAME ${lib_name_static})
