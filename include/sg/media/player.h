/**
 * player.h
 * Simple video player play from filename / stream / memory, without fast go / fast back / drag play.
 */

#ifndef LIBSG_PLAYER_H
#define LIBSG_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_player_real sg_player_t;

sg_player_t *sg_player_create(void);

/**
 * @brief Play local file
 * @param filename Local filename
 * @return 0:OK, other:Error
 */
int sg_player_load_file(sg_player_t *, const char *filename);

/**
 * @brief Play network stream such like rtsp / rtmp / udp
 * @param url Local filename
 * @return 0:OK, other:Error
 */
int sg_player_load_url(sg_player_t *, const char *url);

/**
 * @brief 从bip buffer缓冲区播放，无须用户手动创建bip buffer，player内部创建的
 * @return 0:OK, other:Error
 */
int sg_player_load_buf(sg_player_t *);

/**
 * @brief 向内部的bip buffer缓冲区添加数据
 * @return 0:OK, other:Error
 */
int sg_player_put_buf(sg_player_t *, void *data, size_t size);

int sg_player_put_buf2(sg_player_t *, void *data, size_t size);

/**
 * @brief 获取内部bip buffer缓冲区的未播放数据大小
 * @return 0:OK, other:Error
 */
/* size_t sg_player_unplay_buf_size(sg_player_t *); */

/* async, it will return right now */
int sg_player_play(sg_player_t *);

/**
 * 获取和设置音量, volume取值范围0~100
 */
int sg_player_get_volume(sg_player_t *);

int sg_player_set_volume(sg_player_t *, int volume);

int sg_player_pause(sg_player_t *);

int sg_player_stop(sg_player_t *);

void sg_player_destroy(sg_player_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PLAYER_H */