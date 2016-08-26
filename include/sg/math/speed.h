/**
 * speed.h
 * Speed conversion and counter utility.
 */
 
#ifndef LIBSG_SPEED_H
#define LIBSG_SPEED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_speed_mode {
    SGSPEEDMODE_BIT  = 0, /* bps, Kbps, Mbps, Gbps. For network transfer speed... */
    SGSPEEDMODE_BYTE = 0, /* Bps, KBps, MBps, GBps. For disk copy speed... */
}

enum sg_speed_unit {
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

struct sg_speed_val {
    double             val; /* 1PB会不会溢出?要不要改成uint64 val_int加short val_decimal */
    enum sg_speed_unit unit;
};

typedef struct sg_speed_counter_real sg_speed_counter_t;





struct sg_speed_val *sg_speed_alloc(void);

void sg_speed_set(struct sg_speed_val *, double speed, enum sg_speed_unit);

int sg_speed_conv(struct sg_speed_val *, enum sg_speed_unit new_unit);

/* 以最合适的速度单位转换速度数值, 确保速度单位前的数值在［1，1024］之间，比如1.1Kbps而不是1300bps. */
int sg_speed_conv_auto(struct sg_speed_val *);

/* 格式化输出 */
int sg_speed_fmt_str(struct sg_speed_val *, const char *per_sec_str,
                     char *dst_buf, size_t dst_buf_len);

void sg_speed_free(struct sg_speed_val *);







/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即。
 * stat_duration_ms: 统计时长，最小值100，该范围之内的数据都会纳入统计, 统计时长越长，结果越平滑.
 */
sg_speed_counter_t *sg_speed_counter_open(int stat_duration_ms);

/**
 * function: 接收或者发送数据之后立即把数据量登记进统计工具，注意，要立即。
 */
void sg_speed_counter_reg(sg_speed_counter_t *, size_t byte_size);

/**
 * function: 获取当前速度，get一次计算一次，不get不计算。
 * return: 返回0表示正常，返回其它值表示获取出错。
 */
int sg_speed_counter_read(sg_speed_counter_t *, struct sg_speed_val *speed);

void sg_speed_counter_close(sg_speed_counter_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SPEED_H */