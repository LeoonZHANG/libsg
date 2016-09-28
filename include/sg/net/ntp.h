/**
 * ntp.h
 * Network time protocol client, this module calls system ntp client (ntpd / w32tm).
 */

#ifndef LIBSG_NTP_H
#define LIBSG_NTP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool sg_ntp_sync(const char *server_addr, const int server_port, uint64_t *unix_time_out);

/* try to get time from inside public ntp servers until success */
bool sg_ntp_sync_inside(uint64_t *unix_time_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NTP_H */
