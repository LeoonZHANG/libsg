/**
 * hdw.h
 * Get hard disk and memoryinformation.
 */

#ifndef LIBSG_HDW_H
#define LIBSG_HDW_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool sg_hdw_hd_list_volume(sg_vsstr_list *out);

bool sg_hdw_hd_volume_info(const char *name, uint64_t *used_size, uint64_t *total_size);

uint64_t sg_hdw_memory_total(void);

uint64_t sg_hdw_memory_used(void);

uint64_t sg_hdw_memory_used_by_proc(int proc_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_HDW_H */