/**
 * bloom_filter.h
 * Counting bloom filter and scaling bloom filter data structure.
 */

#ifndef LIBSG_BLOOM_FILTER_H
#define LIBSG_BLOOM_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_counting_bloom_filter_real sg_counting_bloom_filter_t;

sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename);
sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename);
int sg_counting_bloom_filter_add(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_remove(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_contain(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_free(sg_counting_bloom_filter_t *cbf);




typedef struct sg_scaling_bloom_filter_real sg_scaling_bloom_filter_t;

sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename);
sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename);
int sg_scaling_bloom_add(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int sg_scaling_bloom_remove(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int sg_scaling_bloom_contain(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len);
int sg_scaling_bloom_flush(sg_scaling_bloom_filter_t *sbf);
uint64_t sg_scaling_bloom_mem_seqnum(sg_scaling_bloom_filter_t *sbf);
uint64_t sg_scaling_bloom_disk_seqnum(sg_scaling_bloom_filter_t *sbf);
void sg_scaling_bloom_filter_free(sg_scaling_bloom_filter_t *sbf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BLOOM_FILTER_H */