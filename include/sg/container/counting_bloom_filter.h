/**
 * counting_bloom_filter.h
 * Counting bloom filter data structure.
 */

#ifndef LIBSG_COUNTING_BLOOM_FILTER_H
#define LIBSG_COUNTING_BLOOM_FILTER_H

typedef struct sg_counting_bloom_filter_real sg_counting_bloom_filter_t;

sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc(unsigned int capacity, double error_rate, const char *filename);
sg_counting_bloom_filter_t *sg_counting_bloom_filter_alloc_from_file(unsigned int capacity, double error_rate, const char *filename);
int sg_counting_bloom_filter_add(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_remove(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_contain(sg_counting_bloom_filter_t *cbf, const char *key, size_t len);
int sg_counting_bloom_filter_free(sg_counting_bloom_filter_t *cbf);

#endif /* LIBSG_COUNTING_BLOOM_FILTER_H */