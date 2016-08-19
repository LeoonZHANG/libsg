/*
 * dump.h
 * Author: wangwei.
 * Dump toolkit.
 */

#ifndef LIBSG_DUMP_H
#define LIBSG_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SG_DUMP_CORE_SIZE_UNLIMITED -1024

int sg_dump_get_core_size(void);

/* echo 'core.epst.%e.%p.%s.%t' > /proc/sys/kernel/core_pattern */
int sg_dump_get_core_format(void);

int sg_dump_set_core_size(int size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DUMP_H */