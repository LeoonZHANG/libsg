#include <string.h>
#include <stdio.h>
#include <sg/sg.h>
#include <sg/hash/crc.h>
#include "crc_impl.h"

DECLARE_CRC8_TABLE(_table);
static int crc8_table_populated = 0;

void try_initilize_crc8()
{
    if (!crc8_table_populated) {
        /*      poly = x^8 + x^7 + x^6 + x^4 + x^2 + 1
         *
         * For lsb first direction x^7 maps to the lsb. So the polynomial is as below.
         *
         * - lsb first: poly = 10101011(1) = 0xAB
         */
        crc8_populate_lsb(_table, 0xAB);
        crc8_table_populated = 1;
    }
}

int sg_crc_buf(const void *buf, size_t len, enum sg_crc_type type, struct sg_crc_sum *rst)
{
    if (!buf || !rst)
        return -1;

    switch (type) {
    case SGCRCTYPE_CRC_8:
        try_initilize_crc8();
        rst->val_crc_8 = crc8(_table, buf, len, rst->val_crc_8);
        break;
    case SGCRCTYPE_CRC_CCITT:
        rst->val_crc_ccitt = crc_ccitt(rst->val_crc_ccitt, buf, len);
        break;
    case SGCRCTYPE_CRC_16:
        rst->val_crc_16 = crc16(rst->val_crc_16, buf, len);
        break;
    case SGCRCTYPE_CRC_32:
        rst->val_crc_32 = crc32(rst->val_crc_32, buf, len);
        break;
    case SGCRCTYPE_CRC_64:
        rst->val_crc_64 = crc64(rst->val_crc_64, buf, len);
        break;
    }
    return 0;
}

int sg_crc_str(const char *str, enum sg_crc_type type, struct sg_crc_sum *rst)
{
    return sg_crc_buf(str, strlen(str), type, rst);
}

int sg_crc_file(const char *path, enum sg_crc_type type, struct sg_crc_sum *rst)
{
    FILE *file = fopen(path, "rb");
    if (file)
        return -1;

    unsigned char data[1024];
    int bytes;
    while ((bytes = fread(data, sizeof(data[0]), sizeof(data), file)) != 0)
        sg_crc_buf(data, bytes, type, rst);
    fclose(file);

    return 0;
}
