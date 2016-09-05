#include <sg/math/speed.h>
#include <sg/sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* Define time grain as 20ms, this grain should not be too small since time precision and thread swith. */
#define STATISTIC_GRAIN     20

struct sg_speed_counter_real {
    int stat_duration_ms;
    size_t* byte_slots;
    size_t* head;
    size_t* tail;
    unsigned long while_start;
    unsigned long grains_count;
};


struct sg_speed_val *sg_speed_alloc(void)
{
    struct sg_speed_val* result = calloc(1, sizeof(struct sg_speed_val));
    return result;
}

void sg_speed_free(struct sg_speed_val * speed)
{
    free(speed);
}

void sg_speed_set(struct sg_speed_val *speed, double value, enum sg_speed_unit unit)
{
    speed->val = value;
    speed->unit = unit;
}

static int calculate_2_exponent(enum sg_speed_unit old_unit, enum sg_speed_unit new_unit)
{
    const int is_new_unit_in_byte_mode = new_unit & 128;
    const int is_old_unit_in_byte_mode = old_unit & 128;
    int exponent = 0;
    if (is_old_unit_in_byte_mode && !is_new_unit_in_byte_mode)
        exponent += 3;
    else if (!is_old_unit_in_byte_mode && is_new_unit_in_byte_mode)
        exponent -= 3;
    exponent += ((old_unit & 127 - SGSPEEDUNIT_bps) - (new_unit & 127 - SGSPEEDUNIT_bps)) * 10;
    return exponent;
}

int sg_speed_conv(struct sg_speed_val * speed, enum sg_speed_unit new_unit)
{
    int exponent = calculate_2_exponent(speed->unit, new_unit);
    speed->val *= pow(2, exponent);
    speed->unit = new_unit;
    return 0;
}

int sg_speed_conv_auto(struct sg_speed_val *speed)
{
    while (speed->val >= 1024) {
        ++speed->unit;
        speed->val /= 1024;
    }
    return 0;
}

int sg_speed_fmt_str(struct sg_speed_val * speed, const char *per_sec_str, char *dst_buf, size_t dst_buf_len)
{
    const char* unit_strings[] = { "", "K", "M", "G", "T", "P", "Unkown" };
    const int is_unit_in_byte_mode = speed->unit & 128;

    return snprintf(dst_buf, dst_buf_len, "%0.4f %s%s",
                    speed->val,
                    unit_strings[speed->unit & 127 - SGSPEEDUNIT_bps],
                    is_unit_in_byte_mode ? "Bps" : "bps");
}

static size_t counter_slots_capacities(sg_speed_counter_t* counter)
{
    return counter->stat_duration_ms / STATISTIC_GRAIN;
}

sg_speed_counter_t *sg_speed_counter_open(int stat_duration_ms)
{
    if (stat_duration_ms < 100)
        return NULL;

    sg_speed_counter_t* result = malloc(sizeof(sg_speed_counter_t));
    result->stat_duration_ms = stat_duration_ms;
    result->byte_slots = calloc(counter_slots_capacities(result), sizeof(size_t));
    result->head = result->byte_slots;
    result->tail = result->byte_slots;
    result->while_start = sg_boot_time_ms();
    result->grains_count = 0;
    return result;
}

void sg_speed_counter_close(sg_speed_counter_t * counter)
{
    if (counter) {
        if (counter->byte_slots) {
            free(counter->byte_slots);
            counter->byte_slots = NULL;
        }
        free(counter);
    }
}

static size_t counter_slots_size(sg_speed_counter_t* counter)
{
    int distance = counter->tail - counter->head;
    if (distance >= 0)
        return counter->tail - counter->head;
    else
        return counter_slots_capacities(counter) + 1 + distance;
}

static int counter_slots_is_full(sg_speed_counter_t* counter)
{
    return counter_slots_size(counter) == counter_slots_capacities(counter);
}

static void counter_slots_advance(sg_speed_counter_t* counter, size_t** pointer)
{
    ++(*pointer);
    if ((*pointer) >= (counter->byte_slots + counter_slots_capacities(counter)))
        (*pointer) = counter->byte_slots;
}

static void counter_forward(sg_speed_counter_t* counter)
{
    counter_slots_advance(counter, &counter->tail);
    *counter->tail = 0;
    /* The tail hit head, slots must be full, 'cause it's definitely not empty.
       Then advance head. */
    if (counter->tail == counter->head)
        counter_slots_advance(counter, &counter->head);
}

static void try_fast_forward(sg_speed_counter_t * counter)
{
    const unsigned long now = sg_boot_time_ms();
    unsigned long new_grains_count = (now - counter->while_start) / STATISTIC_GRAIN;
    unsigned long elapsed_grains = new_grains_count - counter->grains_count;
    
    /* handle fast-forward, try to set empty slots */
    while (elapsed_grains-- > 0)
        counter_forward(counter);

    counter->grains_count = new_grains_count;
}

void sg_speed_counter_reg(sg_speed_counter_t * counter, size_t byte_size)
{
    try_fast_forward(counter);
    *counter->tail += byte_size;
}

int sg_speed_counter_read(sg_speed_counter_t *counter, struct sg_speed_val *speed)
{
    double total = 0;
    size_t slots_count = counter_slots_size(counter);

    try_fast_forward(counter);

    if (slots_count > 0) {
        for (size_t* p = counter->head; p != counter->tail; counter_slots_advance(counter, &p)) {
            total += *p;
        }
    }
    /* count up the incompleted slot as well */
    total += *counter->tail;

    sg_speed_set(speed, total, SGSPEEDUNIT_Bps);
    sg_speed_conv_auto(speed);
    return 0;
}

