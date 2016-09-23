/**
 * ntp.h
 * Author: wangwei.
 * Network time protocol client.
 */

#ifndef LIBSG_NTP_H
#define LIBSG_NTP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

sg_err_t sg_ntp_get(const char *server_addr, const int server_port, uint64_t *unix_time_out);

/* try to get time from inside public ntp servers until success */
sg_err_t sg_ntp_get_from_inside_server(uint64_t *unix_time_out);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NTP_H */
