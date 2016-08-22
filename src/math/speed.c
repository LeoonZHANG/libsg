#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../../include/sg/math/speed.h"

#define SG_SPEED_COUNTER_SAMPLE_COUNT 16

struct sg_speed_real {
    uint64_t value_Bps;
    float value_non_Bps;
    enum sg_speed_unit unit;
};

struct sg_speed_counter_real {
    int stat_duration_ms;
    int head, tail;
    int byte_slots[SG_SPEED_COUNTER_SAMPLE_COUNT];
};






sg_speed_t *sg_speed_open(void)
{
    struct sg_speed_real *s = (struct sg_speed_real *)malloc(sizeof(struct sg_speed_real));
    return (sg_speed_t *)s;
}

void sg_speed_set_val(float speed, enum sg_speed_unit)
{
    uint64_t val_uint64;
    
    
}

int sg_speed_get_val(sg_speed_t *src, enum sg_speed_unit get_unit,
        float *dst, enum sg_speed_unit *dst_unit, bool *is_dst_decimal_valid)
{
}

int sg_speed_get_val_auto(sg_speed_t *src, enum sg_speed_mode mode,
        float *dst, enum sg_speed_unit *dst_unit, bool *is_dst_decimal_valid)
{
}
        
int sg_speed_get_str(sg_speed_t *src, enum sg_speed_unit dst_unit, const char *per_sec_str,
        char *dst_buf, size_t dst_buf_len)
{
}

int sg_speed_get_str_auto(sg_speed_t *src, enum sg_speed_mode mode, const char *per_sec_str
        char *dst_buf, size_t dst_buf_len)
{
}

void sg_speed_close(sg_speed_t *s)
{
    free(s);
}



sg_speed_counter_t *sg_speed_stat_open(int stat_duration_ms)
{
    sg_speed_counter_t *counter = (sg_speed_counter_t *)malloc(sizeof(sg_speed_counter_t));
    assert(counter);
    memset(counter, 0, sizeof(sg_speed_counter_t));

    counter->stat_duration_ms = stat_duration_ms;
    return counter;
}


void sg_speed_counter_reg(sg_speed_counter_t *counter, size_t byte_size)
{
    counter->byte_slots[counter->tail++] = byte_size;
    counter->tail &= (SG_SPEED_COUNTER_SAMPLE_COUNT - 1);
    if (counter->tail == counter->head)
        counter->head = (counter->head + 1) & (SG_SPEED_COUNTER_SAMPLE_COUNT - 1);
}


int sg_speed_counter_read(sg_speed_counter_t *counter,
                            uint64_t *speed_bps,
                            float *speed_adaptive,
                            enum sg_speed_stat_unit *speed_unit_adaptive)
{
    int i;
    int count = 0;
    int bytes = 0;
    float bps = 0;
    int unit = SGSPEEDSTATUNIT_bps;
    
    // 计算速度
    for (i = counter->head; i != counter->tail; i = (i + 1) & (SG_SPEED_COUNTER_SAMPLE_COUNT - 1)) {
        bytes += counter->byte_slots[i];
        count += 1;
    }
    // 还没有采样
    if (count == 0)
        return -1;

    bps = bytes / (count * counter->stat_duration_ms * 0.001);
    if (speed_bps) {
        *speed_bps = (int) bps;
    }

    // 以合适单位输出
    if (!(speed_adaptive && speed_unit_adaptive))
        return 0;
    for (unit = SGSPEEDSTATUNIT_MIN; unit <= SGSPEEDSTATUNIT_MAX; ++unit) {
        if (bps < 1024)
            goto done;
        else
            bps /= 1024;
    }

    done:
    *speed_adaptive = bps;
    *speed_unit_adaptive = unit;
    return 0;
}


void sg_speed_counter_close(sg_speed_counter_t *counter)
{
    assert(counter);
    free(counter);
}
