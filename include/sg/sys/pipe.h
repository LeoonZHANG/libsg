/**
 * pipe.h
 * Pipe for multiple platforms.
 * 基于这个来实现 https://github.com/apache/apr/blob/trunk/include/apr_file_io.h,接口也要微调
 * 注意阻塞类型对使用,如果在不同平台上有不一样的特性,请把差异屏蔽掉
 */

#ifndef LIBSG_PIPE_H
#define LIBSG_PIPE_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_pipe_type {
    SGPIPETYPE_ANON  = 0, /* anonymous pipe */
    SGPIPETYPE_NAMED = 0, /* named pipe */
};

enum sg_pipe_blocking_type {
    SGPIPEBLOCKINGTYPE_FULL_BLOCK = 0,
    SGPIPEBLOCKINGTYPE_READ_BLOCK,
    SGPIPEBLOCKINGTYPE_WRITE_BLOCK,
    SGPIPEBLOCKINGTYPE_FULL_NONBLOCK
};

typedef struct sg_pipe_real sg_pipe_t;

sg_pipe_t *sg_pipe_create_anon(enum sg_pipe_blocking_type type, int **in, int **out);

sg_pipe_t *sg_pipe_create_named(enum sg_pipe_blocking_type type, const char *filename);

int sg_pipe_set_timeout(sg_pipe_t *self, int ms);

int sg_pipe_read(sg_pipe_t *self, void *read_buf, int read_size);

int sg_pipe_write(sg_pipe_t *self, void *write_buf, int write_size);

void sg_pipe_destroy(sg_pipe_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PIPE_H */
