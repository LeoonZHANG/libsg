/**
 * crc.h
 * Common crc.
 */

#ifndef LIBSG_CRC_H
#define LIBSG_CRC_H

#include <stdlib.h>
#include <stdint.h>

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

struct sg_crc_sum {
    union {
        uint8_t   val_crc_8;
        uint16_t  val_crc_ccitt;
        uint16_t  val_crc_16;
        uint32_t  val_crc_32;
        uint64_t  val_crc_64;
    };
};

int sg_crc_buf(const void *buf, size_t len, enum sg_crc_type type, struct sg_crc_sum *rst);
int sg_crc_str(const char *str, enum sg_crc_type type, struct sg_crc_sum *rst);
int sg_crc_file(const char *path, enum sg_crc_type type, struct sg_crc_sum *rst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CRC_H */
