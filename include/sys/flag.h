/*
 * flag.h
 * Author: wangwei.
 * Run flag for thread.
 */

#ifndef LIBSG_FLAG_H
#define LIBSG_FLAG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct flag_body sg_flag_t;

/* Initialize a flag structure pointer. Thread safe. */
sg_flag_t *sg_flag_create(void);

/* Return 'value' member in flag body. */
int sg_flag_read(sg_flag_t *f);

/* Rewrite 'value' member in flag body. */
void sg_flag_write(sg_flag_t *f, int value);

/* When any thread is trying flag_read / flag_write to a flag,
 * DO NOT call flag_destroy with it. */
void sg_flag_destroy(sg_flag_t **f);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_FLAG_H */