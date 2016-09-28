/**
 * dns.h
 * DNS client based on mongoose.
 */

#ifndef LIBSG_DNS_H
#define LIBSG_DNS_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool sg_dns_get(const char *domain, sg_vsstr_t *out_ip);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DNS_H */