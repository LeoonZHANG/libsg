/**
 * bloom_filter.h
 * Counting bloom filter and scaling bloom filter data structure.
 */

/**
 * bloom_filter: 基于https://github.com/waruqi/tbox的bloom filter模块
 * conting_bloom_filter和scaling_bloom_filter: 基于https://github.com/bitly/dablooms/实现
 * https://github.com/bitly/dablooms/它是支持linux/unix的, 请写一个fork,少数几个linux的api增加对windows的同等支持,并提交官方PR。
 * 疑问,add、remove喝contain接口中的len,在有的实现中有,有的实现中没有,到底需要吗?
 */

#ifndef LIBSG_BLOOM_FILTER_H
#define LIBSG_BLOOM_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_bloom_filter_real sg_bloom_filter_t;

sg_bloom_filter_t *sg_bloom_filter_alloc(...);
sg_bloom_filter_t *sg_bloom_filter_alloc_from_file();
int sg_bloom_filter_add(sg_bloom_filter_t *cbf, const char *key, size_t len);
int sg_bloom_filter_contain(sg_bloom_filter_t *cbf, const char *key, size_t len);
int sg_bloom_filter_free(sg_bloom_filter_t *cbf);

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
int sg_scaling_bloom_filter_add(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int sg_scaling_bloom_filter_remove(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len, uint64_t id);
int sg_scaling_bloom_filter_contain(sg_scaling_bloom_filter_t *sbf, const char *key, size_t len);
int sg_scaling_bloom_filter_flush(sg_scaling_bloom_filter_t *sbf);
uint64_t sg_scaling_bloom_filter_mem_seqnum(sg_scaling_bloom_filter_t *sbf);
uint64_t sg_scaling_bloom_filter_disk_seqnum(sg_scaling_bloom_filter_t *sbf);
void sg_scaling_bloom_filter_free(sg_scaling_bloom_filter_t *sbf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BLOOM_FILTER_H */