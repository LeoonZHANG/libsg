/*
 * sg.h
 * Author: wangwei.
 * Libsg public module.
 */

#ifndef LIBSG_SG_H
#define LIBSG_SG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* register libsg with key */
int sg_register(const char *key);

/* check expiration date*/
int sg_check_expiration(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SG_H */
