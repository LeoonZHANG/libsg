/**
 * speed_stat.h
 * Speed statistics tool.
 */
 
#ifndef LIBSG_SPEED_STAT_H
#define LIBSG_SPEED_STAT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum sg_speed_stat_unit {
    SGSPEEDSTATUNIT_MIN  = 0,
    SGSPEEDSTATUNIT_bps  = 0,
    SGSPEEDSTATUNIT_Kbps = 1,
    SGSPEEDSTATUNIT_Mbps = 2,
    SGSPEEDSTATUNIT_Gbps = 3,
    SGSPEEDSTATUNIT_MAX  = 3,
}

typedef struct sg_speed_stat_real sg_speed_stat_t;

/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即。
 * stat_duration_ms: 统计时长，最小值100，该范围之内的数据都会纳入统计, 统计时长越长，结果越平滑.
 */
sg_speed_stat_t *sg_speed_stat_open(int stat_duration_ms);

/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即，否则影响速度统计精度。
 */
void sg_speed_add_data_size(sg_speed_stat_t *, size_t);

/**
 * function: 获取当前速度，get一次计算一次，不get不计算。
 * @speed_bps: 以bps为单位输出的速度值。
 * @speed_adaptive: 以最合适的速度单位输出的速度数值, 确保速度单位前的数值在［1，1024］之间，比如1.1Kbps而不是1300bps.
 * @speed_unit_adaptive: 最合适的速度单位.
 * return: 返回0表示正常，返回其它值表示出错。
 */
int sg_speed_stat_get_speed(sg_speed_stat_t *, uint64_t *speed_bps,
        float *speed_adaptive, enum sg_speed_stat_unit *speed_unit_adaptive);

void sg_speed_stat_close(sg_speed_stat_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SPEED_STAT_H */