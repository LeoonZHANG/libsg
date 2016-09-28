/**
 * hd.h
 * Get hard disk information such like volume-name/ volume-format / size / used size.
 */

#ifndef LIBSG_HD_H
#define LIBSG_HD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool sg_hd_list_volume(sg_vsstr_list *out);

bool sg_hd_volume_info(const char *name, uint64_t *used);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_HD_H */