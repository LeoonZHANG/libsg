/*
 * url.h
 * Author: wangwei.
 * URL parser and packer.
 */

#ifndef LIBSG_URL_H
#define LIBSG_URL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool sg_url_is_local_filename(const char *url);

bool sg_url_is_network_url(const char *url);

SG_PROTOCOL sg_url_parse_protocol(const char *url);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_URL_H */
