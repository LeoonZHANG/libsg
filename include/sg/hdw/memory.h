/**
 * memory.h
 * Get memory information.
 */

#ifndef LIBSG_MEMORY_H
#define LIBSG_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint64_t sg_memory_total(void);

uint64_t sg_memory_used(void);

uint64_t sg_memory_used_by_proc(int proc_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MEMORY_H */