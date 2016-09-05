#include <stdio.h>
#include <sg/media/player.h>
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_media_player.h>

#define PLAY_BUFFER_SIZE 4096

struct sg_player_real {
    libvlc_instance_t       *inst;
    libvlc_media_t          *media;
    libvlc_media_player_t   *player;
    int                     pipefd[2];
};

enum sg_player_load_type {
    SGPLAYERLOADTYPE_MIN      = 0,
    SGPLAYERLOADTYPE_FILENAME = 0,
    SGPLAYERLOADTYPE_URL      = 1,
    SGPLAYERLOADTYPE_BUF      = 2,
    SGPLAYERLOADTYPE_MAX      = 2
};

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
        if (pipe(pl->pipefd) != 0) {
            printf("pipe create error\n", (FILE *)load_src);
            goto error;
        }
        pl->media = libvlc_media_new_fd(pl->inst, pl->pipefd[0]);
        if (!pl->media) {
            printf("create media from pipe error\n", (FILE *)load_src);
            goto error;
        }
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

    if (write((char *)data, pl->pipefd[1], size) == 0) {
        close(pl->pipefd[1]);
        printf("pipe in vlc write error\n");
        return -1;
    } else
        return 0;
}

int sg_player_play(sg_player_t *p)
{
    struct sg_player_real *pl = (struct sg_player_real *)p;

    libvlc_media_player_play(pl->player);
    return 0;
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
    if (pl->pipefd[0])
        close(pl->pipefd[0]);
    if (pl->pipefd[1])
        close(pl->pipefd[1]);
    free(pl);
}