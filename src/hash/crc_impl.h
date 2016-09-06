#ifndef __SG_HASH_CRC_IMPL_H__
#define __SG_HASH_CRC_IMPL_H__

#include <stdlib.h>
#include <stdint.h>

/* required table size for crc8 algorithm */
#define CRC8_TABLE_SIZE                 256

/* helper macro assuring right table size is used */
#define DECLARE_CRC8_TABLE(_table) \
        static uint8_t _table[CRC8_TABLE_SIZE]

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void crc8_populate_msb(uint8_t table[CRC8_TABLE_SIZE], uint8_t polynomial);
void crc8_populate_lsb(uint8_t table[CRC8_TABLE_SIZE], uint8_t polynomial);
uint8_t crc8(const uint8_t table[CRC8_TABLE_SIZE], const uint8_t *pdata, size_t nbytes, uint8_t crc);

uint16_t crc_ccitt(uint16_t crc, const uint8_t *buffer, size_t len);
uint16_t crc16(uint16_t crc, const uint8_t *buffer, size_t len);
uint32_t crc32(uint32_t seed, const unsigned char *buf, size_t len);
uint64_t crc64(uint64_t seed, const unsigned char *data, size_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
