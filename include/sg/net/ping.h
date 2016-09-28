/**
 * ping.h
 * This module calls system ping tool.
 */

#ifndef LIBSG_PING_H
#define LIBSG_PING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void sg_ping(const char *dst_host, const int keep_sec,
             uint8_t *avg_packet_loss, uint32_t *avg_delay_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PING_H */