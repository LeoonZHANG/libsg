/*
 * pipe.h
 * Author: wangwei.
 * Pipe for multiple platforms.
 */

#ifndef LIBSG_PIPE_H
#define LIBSG_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_pipe_type {
    SGPIPETYPE_ANON  = 0, /* anonymous pipe */
    SGPIPETYPE_NAMED = 0, /* named pipe */
};

typedef struct sg_pipe_real sg_pipe_t;

sg_pipe_t *sg_pipe_create_anon(void);

sg_pipe_t *sg_pipe_create_named(const char *pipe_name);

int sg_pipe_set_name(sg_pipe_t *me, const char *name);

int sg_pipe_read(sg_pipe_t *me, void *read_buf, int read_size);

int sg_pipe_write(sg_pipe_t *me, void *write_buf, int write_size);

void sg_pipe_destroy(sg_pipe_t *me);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PIPE_H */
