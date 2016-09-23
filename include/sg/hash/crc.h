/**
 * crc.h
 * Common crc.
 */

#ifndef LIBSG_CRC_H
#define LIBSG_CRC_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_crc_real sg_crc_t;

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

sg_crc_t *sg_crc_start(enum sg_crc_type);
int sg_crc_update(sg_crc_t *self, void *data, uint64_t size);
int sg_crc_finish(sg_crc_t *self, struct sg_crc_sum *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CRC_H */
