/**
 * msgpack.h
 * 把msgpack的常用接口包装起来即可,可以参考json和xml的接口设计
 */

#ifndef LIBSG_MSGPACK_H
#define LIBSG_MSGPACK_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 把json转换成msgpack, 官方demo有实现 */
sg_msgpack_t *sg_magpack_alloc_from_json(const char *json_str, uint64_t len);

sg_msgpack_t *sg_magpack_alloc_from_json2(sg_json_doc_t *json_doc);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MSGPACK_H */
