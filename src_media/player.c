#include <stdio.h>
#include <sg/media/player.h>
#include <sg/container/bip_buf.h>
#include <sg/sys/thread.h>
#include <sg/sys/mutex.h>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_media_player.h>

#define PLAY_BUFFER_SIZE 4096

struct sg_player_real {
    libvlc_instance_t       *inst;
    libvlc_media_t          *media;
    libvlc_media_player_t   *player;
    int                     fd[2];
    sg_bip_buf_t            *io_buf; /* play buffer */
    sg_mutex_t              *io_buf_mtx;
    sg_thread_t             *io_thread;
};

enum sg_player_load_type {
    SGPLAYERLOADTYPE_MIN      = 0,
    SGPLAYERLOADTYPE_FILENAME = 0,
    SGPLAYERLOADTYPE_URL      = 1,
    SGPLAYERLOADTYPE_BUF      = 2,
    SGPLAYERLOADTYPE_MAX      = 2
};

static void io_thread_func(void *param)
{
    struct sg_player_real *pl = (struct sg_player_real *)param;
    int buf_get_size = 0;
    unsigned char buf_tmp[1024];

    while (1) {
        sg_mutex_lock(pl->io_buf_mtx);
        buf_get_size = sg_bip_buf_used_size(pl->io_buf);
        sg_mutex_unlock(pl->io_buf_mtx);
        if (buf_get_size == 0) {
            usleep(1);
            continue;
        }
        if (buf_get_size > 1024)
            buf_get_size = 1024;

        sg_mutex_lock(pl->io_buf_mtx);
        memcpy(buf_tmp, sg_bip_buf_get(pl->io_buf, buf_get_size), buf_get_size);
        sg_mutex_unlock(pl->io_buf_mtx);
        write(pl->fd[1], buf_tmp, buf_get_size);
    }
}

/*
ssize_t libvlc_media_read_cb(void *opaque, unsigned char *buf, size_t len)
{
	sg_bip_buf_t* tmp=(sg_bip_buf_t*)opaque;
	len=sg_bip_buf_used_size(tmp);
	buf=sg_bip_buf_get(tmp,len);
	return len;

}

int libvlc_media_open_cb(void *opaque, void **datap, uint64_t *sizep)
{
	return 0;
}
int libvlc_media_seek_cb(void *opaque, uint64_t offset)
{
	return 0;
}
int libvlc_media_close_cb(void*opaque)
{
	return 0;
}
*/

static int sg_player_load(sg_player_t *p, const void *load_src, enum sg_player_load_type load_type)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    if (load_type < SGPLAYERLOADTYPE_MIN || load_type > SGPLAYERLOADTYPE_MAX)
        goto error;
    if (pl->player) {
        libvlc_media_player_release(pl->player);
        pl->player = NULL;
    }

    if (load_type == SGPLAYERLOADTYPE_FILENAME) {
        pl->media = libvlc_media_new_path(pl->inst, (const char *)load_src);
        if (!pl->media) {
            printf("file %s open error\n", (const char *)load_src);
            goto error;
        }
    } else if (load_type == SGPLAYERLOADTYPE_URL) {
        pl->media = libvlc_media_new_location(pl->inst, (const char *)load_src);
        if (!pl->media) {
            printf("url %s open error\n", (const char *)load_src);
            goto error;
        }
    } else {
        pl->io_buf = sg_bip_buf_create(102400);
        if (!pl->io_buf) {
            printf("play buffer create error\n");
            goto error;
        }
        pl->io_buf_mtx = sg_mutex_create();
        if (!pl->io_buf_mtx) {
            printf("play buffer mutex create error\n");
            goto error;
        }
        if (pipe(pl->fd) != 0) {
            printf("pipe create error\n");
            goto error;
        }
        pl->media = libvlc_media_new_fd(pl->inst, pl->fd[0]);
        if (!pl->media) {
            printf("create media from pipe error\n");
            goto error;
        }
        printf("play load buf success, fd[0]:%d, fd[1]:%d\n", pl->fd[0], pl->fd[1]);
        pl->io_thread = sg_thread_alloc(io_thread_func, (void *)pl);
    }

    pl->player = libvlc_media_player_new_from_media(pl->media);
    if(!pl->player) {
        printf("vlc player create error\n");
        goto error;
    }

    succeed:
    return 0;

    error:
    if (pl->player)
        libvlc_media_player_release(pl->player);
    if (pl->inst)
        libvlc_release(pl->inst);
    if (pl->media)
        libvlc_media_release(pl->media);
    if (pl->io_buf)
        sg_bip_buf_destroy(pl->io_buf);
    if (pl->io_buf_mtx)
        sg_mutex_destroy(pl->io_buf_mtx);
    exit(-1);
    return -1;
}

sg_player_t *sg_player_create(void)
{
    struct sg_player_real *p = (struct sg_player_real *)malloc(sizeof(struct sg_player_real));
    if (!p)
        return NULL;

    memset(p, 0, sizeof(struct sg_player_real));

    p->inst = libvlc_new(0,NULL);

    if (!p->inst)
        return NULL;

    return (sg_player_t *)p;
}

int sg_player_load_file(sg_player_t *p, const char *filename)
{
    return sg_player_load(p, (const void *)filename, SGPLAYERLOADTYPE_FILENAME);
}

int sg_player_load_url(sg_player_t *p, const char *url)
{
    return sg_player_load(p, (const void *)url, SGPLAYERLOADTYPE_URL);
}

int sg_player_load_buf(sg_player_t *p)
{
    return sg_player_load(p, (const void *)NULL, SGPLAYERLOADTYPE_BUF);
}

int sg_player_put_buf(sg_player_t *p, void *data, size_t size)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;
    int write_len_all = 0;
    int write_len_once = 0;

    while (write_len_all < size) {
        write_len_once = write(pl->fd[1], data, size);
        if (write_len_once < size)
            printf("write once small than total\n");
        if (write_len_once <= 0) {
            printf("pipe in vlc write error\n");
            return -1;
        } else
            write_len_all += write_len_once;
    }
    return 0;
}

int sg_player_put_buf2(sg_player_t *p, void *data, size_t size)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    sg_mutex_lock(pl->io_buf_mtx);
    sg_bip_buf_put(pl->io_buf, data, size);
    sg_mutex_unlock(pl->io_buf_mtx);

    return 0;
}

int sg_player_play(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;
    int ret;

    ret = libvlc_media_player_play(pl->player);
    printf("vlc player play %s\n", ret == 0 ? "success" : "error");
    return ret;
}

int sg_player_get_volume(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    return libvlc_audio_get_volume(pl->player);
}

int sg_player_set_volume(sg_player_t *p, int volume)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    return libvlc_audio_set_volume(pl->player, volume);
}

int sg_player_pause(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    libvlc_media_player_pause(pl->player);
    return 0;
}

int sg_player_stop(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    libvlc_media_player_stop(pl->player);
    return 0;
}

void sg_player_destroy(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;
    if (!pl)
        return;

    if (pl->media)
        libvlc_media_release(pl->media);
    if (pl->player)
        libvlc_media_player_release(pl->player);
    if (pl->inst)
        libvlc_release(pl->inst);
    if (pl->fd[0])
        close(pl->fd[0]);
    if (pl->fd[1])
        close(pl->fd[1]);
    if (pl->io_buf)
        sg_bip_buf_destroy(pl->io_buf);
    if (pl->io_buf_mtx)
        sg_mutex_destroy(pl->io_buf_mtx);
    free(pl);
}