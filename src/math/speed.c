#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../../include/sg/math/speed.h"

#if 0
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
#endif