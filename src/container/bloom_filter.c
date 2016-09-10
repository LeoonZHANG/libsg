#include <sg/container/bloom_filter.h>
#include <tbox/tbox.h>
#include <tbox/container/bloom_filter.h>

/****************************************************************
 * regular bloom_filter
 ***************************************************************/
struct sg_bloom_filter_real {
    tb_bloom_filter_ref_t pimpl;
    tb_element_t element;
};

static int tb_initilized = 0;

static int ensure_tb_initilized()
{
    if (!tb_initilized)
        if (tb_init(tb_null, tb_null))
            tb_initilized = 1;
    // FIXME: yes, we won't call tb_exit()
    return tb_initilized;
}

static tb_element_t type_to_element(enum sg_bloom_element_type type)
{
    switch (type) {
    case SGBLOOM_ELEMENT_TYPE_UINT8:
        return tb_element_uint8();
    case SGBLOOM_ELEMENT_TYPE_UINT16:
        return tb_element_uint16();
    case SGBLOOM_ELEMENT_TYPE_UINT32:
        return tb_element_uint32();
    case SGBLOOM_ELEMENT_TYPE_UINT64:
        return tb_element_long();
    case SGBLOOM_ELEMENT_TYPE_CSTRING:
        return tb_element_str(tb_true);
    default:
        return tb_element_null();
    }
}

sg_bloom_filter_t *sg_bloom_filter_alloc(unsigned int capacity, enum sg_bloom_element_type type)
{
    if (!ensure_tb_initilized())
        return NULL;

    tb_element_t element = type_to_element(type);
    if (element.type == TB_ELEMENT_TYPE_NULL)
        return NULL;

    tb_bloom_filter_ref_t pimpl = tb_bloom_filter_init(TB_BLOOM_FILTER_PROBABILITY_0_001, 3, capacity, element);
    //tb_bloom_filter_ref_t pimpl = tb_bloom_filter_init(TB_BLOOM_FILTER_PROBABILITY_0_01, 3, capacity, tb_element_str(tb_true));
    if (!pimpl)
        return NULL;

    sg_bloom_filter_t* result = calloc(1, sizeof(sg_bloom_filter_t));
    if (result) {
        result->element = element;
        result->pimpl = pimpl;
    }
    return result;
}

int sg_bloom_filter_free(sg_bloom_filter_t *bloom)
{
    if (bloom) {
        if (bloom->pimpl) {
            tb_bloom_filter_exit(bloom->pimpl);
            bloom->pimpl = NULL;
        }
    }
    return 0;
}

int sg_bloom_filter_add(sg_bloom_filter_t *bloom, const void *key)
{
    return tb_bloom_filter_set(bloom->pimpl, key);
}

int sg_bloom_filter_contain(sg_bloom_filter_t *bloom, const void *key)
{
    return tb_bloom_filter_get(bloom->pimpl, key);
}

#if !defined(WIN32)
#include <dablooms.h>

/****************************************************************
 * counting_bloom
 ***************************************************************/
struct sg_counting_bloom_filter_real {
    counting_bloom_t *pimpl;
};

sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename)
{
    sg_counting_bloom_filter_t* result = calloc(1, sizeof(sg_counting_bloom_filter_t));
    if (!(result->pimpl = new_counting_bloom(capacity, error_rate, filename))) {
        free(result);
        result = NULL;
    }
    return result;
}

sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename)
{
    sg_counting_bloom_filter_t* result = calloc(1, sizeof(sg_counting_bloom_filter_t));
    if (!(result->pimpl = new_counting_bloom_from_file(capacity, error_rate, filename))) {
        free(result);
        result = NULL;
    }
    return result;
}

int sg_counting_bloom_filter_free(sg_counting_bloom_filter_t *bloom)
{
    if (bloom) {
        if (bloom->pimpl) {
            free_counting_bloom(bloom->pimpl);
            bloom->pimpl = NULL;
        }
    }
    return 0;
}

int sg_counting_bloom_filter_add(sg_counting_bloom_filter_t *bloom, const char *key, size_t len)
{
    return counting_bloom_add(bloom->pimpl, key, len);
}

int sg_counting_bloom_filter_remove(sg_counting_bloom_filter_t *bloom, const char *key, size_t len)
{
    return counting_bloom_remove(bloom->pimpl, key, len);
}

int sg_counting_bloom_filter_contain(sg_counting_bloom_filter_t *bloom, const char *key, size_t len)
{
    return counting_bloom_check(bloom->pimpl, key, len);
}


/****************************************************************
 * scaling_bloom
 ***************************************************************/
struct sg_scaling_bloom_filter_real {
    scaling_bloom_t* pimpl;
};

sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename)
{
    sg_scaling_bloom_filter_t* result = calloc(1, sizeof(sg_scaling_bloom_filter_t));
    if (!(result->pimpl = new_scaling_bloom(capacity, error_rate, filename))) {
        free(result);
        result = NULL;
    }
    return result;
}

sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename)
{
    sg_scaling_bloom_filter_t* result = calloc(1, sizeof(sg_scaling_bloom_filter_t));
    if (!(result->pimpl = new_scaling_bloom_from_file(capacity, error_rate, filename))) {
        free(result);
        result = NULL;
    }
    return result;
}

void sg_scaling_bloom_filter_free(sg_scaling_bloom_filter_t *bloom)
{
    if (bloom) {
        if (bloom->pimpl) {
            free_scaling_bloom(bloom->pimpl);
            bloom->pimpl = NULL;
        }
    }
}

int sg_scaling_bloom_filter_add(sg_scaling_bloom_filter_t *bloom, const char *key, size_t len, uint64_t id)
{
    return scaling_bloom_add(bloom->pimpl, key, len, id);
}

int sg_scaling_bloom_filter_remove(sg_scaling_bloom_filter_t *bloom, const char *key, size_t len, uint64_t id)
{
    return scaling_bloom_remove(bloom->pimpl, key, len, id);
}

int sg_scaling_bloom_filter_contain(sg_scaling_bloom_filter_t *bloom, const char *key, size_t len)
{
    return scaling_bloom_check(bloom->pimpl, key, len);
}

int sg_scaling_bloom_filter_flush(sg_scaling_bloom_filter_t *bloom)
{
    return scaling_bloom_flush(bloom->pimpl);
}

uint64_t sg_scaling_bloom_filter_mem_seqnum(sg_scaling_bloom_filter_t *bloom)
{
    return scaling_bloom_mem_seqnum(bloom->pimpl);
}

uint64_t sg_scaling_bloom_filter_disk_seqnum(sg_scaling_bloom_filter_t *bloom)
{
    return scaling_bloom_disk_seqnum(bloom->pimpl);
}
#endif
