#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../../include/sg/net/speed_stat.h"

#define SG_ETP_SPEED_STAT_SAMPLE_COUNT 16
struct sg_speed_stat_real {
    int stat_duration_ms;
    int head, tail;
    int byte_slots[SG_ETP_SPEED_STAT_SAMPLE_COUNT];

};


sg_speed_stat_t *sg_speed_stat_open(int stat_duration_ms)
{
    sg_speed_stat_t *speed_stat = malloc(sizeof(sg_speed_stat_t));
    assert(speed_stat);
    memset(speed_stat, 0, sizeof(sg_speed_stat_t));

    speed_stat->stat_duration_ms = stat_duration_ms;
    return speed_stat;
}


void sg_speed_add_data_size(sg_speed_stat_t *speed_stat, size_t bytes)
{
    speed_stat->byte_slots[speed_stat->tail++] = bytes;
    speed_stat->tail &= (SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1);
    if (speed_stat->tail == speed_stat->head)
        speed_stat->head = (speed_stat->head + 1) & (SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1);
}


int sg_speed_stat_get_speed(sg_speed_stat_t *speed_stat,
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
    for (i = speed_stat->head; i != speed_stat->tail; i = (i + 1) & (SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1)) {
        bytes += speed_stat->byte_slots[i];
        count += 1;
    }
    // 还没有采样
    if (count == 0)
        return -1;

    bps = bytes / (count * speed_stat->stat_duration_ms * 0.001);
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


void sg_speed_stat_close(sg_speed_stat_t *speed_stat)
{
    assert(speed_stat);
    free(speed_stat);
}
