/**
 * scaling_bloom_filter.h
 * Scaling bloom filter data structure.
 */

#ifndef LIBSG_SCALING_BLOOM_FILTER_H
#define LIBSG_SCALING_BLOOM_FILTER_H

typedef struct sg_scaling_bloom_filter_real sg_scaling_bloom_filter_t;


sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename);
sg_scaling_bloom_filter_t *sg_scaling_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename);
int scaling_bloom_add(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int scaling_bloom_remove(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int scaling_bloom_contain(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len);
int scaling_bloom_flush(sg_scaling_bloom_filter_t *sbf);
uint64_t scaling_bloom_mem_seqnum(sg_scaling_bloom_filter_t *sbf);
uint64_t scaling_bloom_disk_seqnum(sg_scaling_bloom_filter_t *sbf);
void sg_scaling_bloom_filter_free(sg_scaling_bloom_filter_t *sbf);

#endif /* LIBSG_COUNTING_BLOOM_FILTER_H */