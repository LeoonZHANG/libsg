/**
 * speed.h
 * Speed conversion and statistics utility.
 */
 
#ifndef LIBSG_SPEED_H
#define LIBSG_SPEED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

sypedef enum sg_speed_mode {
    SGSPEEDMODE_BIT = 0,  /* bps, Kbps, Mbps, Gbps. For network transfer speed... */
    SGSPEEDMODE_BYTE = 0, /* Bps, KBps, MBps, GBps. For disk copy speed... */
}

typedef enum sg_speed_unit {
    SGSPEEDUNIT_ADAPTIVE = -1,
    SGSPEEDUNIT_MIN  = 0,
    /* bit mode */
    SGSPEEDUNIT_bps  = 0,
    SGSPEEDUNIT_Kbps = 1,
    SGSPEEDUNIT_Mbps = 2,
    SGSPEEDUNIT_Gbps = 3,
    /* byte mode */
    SGSPEEDUNIT_Bps  = 4,
    SGSPEEDUNIT_KBps = 5,
    SGSPEEDUNIT_MBps = 6,
    SGSPEEDUNIT_GBps = 7,
    SGSPEEDUNIT_MAX  = 7
};


typedef struct sg_speed_real sg_speed_t;
typedef struct sg_speed_stat_real sg_speed_stat_t;


sg_speed_t *sg_speed_open(sg_speed_mode mode);

void sg_speed_set(float speed, enum sg_speed_unit);

int sg_speed_get(sg_speed_t *src, enum sg_speed_unit get_unit,
        float *dst, enum sg_speed_unit *dst_unit, bool *is_dst_decimal_valid);

int sg_speed_fmt(sg_speed_t *src, enum sg_speed_unit dst_unit, char **dst_str);

void sg_speed_close(sg_speed_t *);







/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即。
 * stat_duration_ms: 统计时长，最小值100，该范围之内的数据都会纳入统计, 统计时长越长，结果越平滑.
 */
sg_speed_stat_t *sg_speed_stat_open(int stat_duration_ms);

/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即。
 */
void sg_speed_stat_add_data_size(sg_speed_stat_t *, size_t byte_size);

/**
 * function: 获取当前速度，get一次计算一次，不get不计算。
 * @speed_bps: 以bps为单位输出的速度值。
 * @speed_adaptive: 以最合适的速度单位输出的速度数值, 确保速度单位前的数值在［1，1024］之间，比如1.1Kbps而不是1300bps.
 * @speed_unit_adaptive: 最合适的速度单位.
 * return: 返回0表示正常，返回其它值表示获取出错。
 */
int sg_speed_stat_get_speed(sg_speed_stat_t *, sg_speed_t *);

void sg_speed_stat_close(sg_speed_stat_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SPEED_H */