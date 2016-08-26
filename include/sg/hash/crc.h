/**
 * crc.h
 * Common crc.
 */

#ifndef LIBSG_CRC_H
#define LIBSG_CRC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_crc_type {
    SGCRCTYPE_CRC_8      = 0,
    SGCRCTYPE_CRC_CCITT  = 1,
    SGCRCTYPE_CRC_16     = 2,
    SGCRCTYPE_CRC_32     = 3,
    SGCRCTYPE_CRC_64     = 4
};

/* 是不是可以用union? */
struct sg_crc_sum {
    uint8_t   val_crc_8;
    uint16_t  val_crc_ccitt;
    uint16_t  val_crc_16;
    uint32_t  val_crc_32;
    uint64_t  val_crc_64;
};

/* 如果是对一个很大的内存放不下的文件,是否可以连续调用这个接口实现对大文件的crc计算?
 * 应该是可以的, 如果不可以,那还需要增加crc的接口,类似md模块 */
int sg_crc_buf(void *buf, size_t len, enum sg_crc_type type, struct sg_crc_sum *rst);
int sg_crc_str(const char *str, enum sg_crc_type type, struct sg_crc_sum *rst);
int sg_crc_file(const char *path, enum sg_crc_type type, struct sg_crc_sum *rst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CRC_H */