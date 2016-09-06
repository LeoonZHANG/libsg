#include <sg/container/bloom_filter.h>
#include <gtest/gtest.h>
#include <stdio.h>

#define CAPACITY 100000
#define ERROR_RATE .05
#define BLOOM_FILE "./bloom.bin"

TEST(test_bloom, all)
{
    sg_bloom_filter_t *bloom;
    ASSERT_TRUE(bloom = sg_bloom_filter_alloc(CAPACITY, SGBLOOM_ELEMENT_TYPE_CSTRING))
        << "ERROR: Could not create bloom filter";

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        sg_bloom_filter_add(bloom, word);
    }

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        EXPECT_TRUE(sg_bloom_filter_contain(bloom, word))
            << "missing: " << word;
    }

    sg_bloom_filter_free(bloom);
}

TEST(test_counting_bloom, remove_reopen_and_accuracy)
{
    if (FILE* fp = fopen(BLOOM_FILE, "r")) {
        fclose(fp);
        remove(BLOOM_FILE);
    }

    sg_counting_bloom_filter_t *bloom;
    ASSERT_TRUE(bloom = sg_counting_bloom_filter_alloc(CAPACITY, ERROR_RATE, BLOOM_FILE))
        << "ERROR: Could not create bloom filter";

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        sg_counting_bloom_filter_add(bloom, word, strlen(word));
    }

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        EXPECT_TRUE(sg_counting_bloom_filter_contain(bloom, word, strlen(word)))
            << "missing: " << word;
        if (i % 5 == 0) {
            sg_counting_bloom_filter_remove(bloom, word, strlen(word));
        }
    }

    sg_counting_bloom_filter_free(bloom);

    bloom = sg_counting_bloom_filter_alloc_from_file(CAPACITY, ERROR_RATE, BLOOM_FILE);

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        int key_removed = (i % 5 == 0);
        EXPECT_FALSE(!key_removed && !sg_counting_bloom_filter_contain(bloom, word, strlen(word)))
            << "false negative occurred";
    }

    sg_counting_bloom_filter_free(bloom);
}


TEST(test_scaling_bloom, remove_reopen_and_accuracy)
{
    if (FILE* fp = fopen(BLOOM_FILE, "r")) {
        fclose(fp);
        remove(BLOOM_FILE);
    }

    sg_scaling_bloom_filter_t *bloom;
    ASSERT_TRUE(bloom = sg_scaling_bloom_filter_alloc(CAPACITY, ERROR_RATE, BLOOM_FILE))
        << "ERROR: Could not create bloom filter";

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        sg_scaling_bloom_filter_add(bloom, word, strlen(word), i);
    }

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        EXPECT_TRUE(sg_scaling_bloom_filter_contain(bloom, word, strlen(word)))
            << "missing: " << word;
        if (i % 5 == 0) {
            sg_scaling_bloom_filter_remove(bloom, word, strlen(word), i);
        }
    }

    sg_scaling_bloom_filter_free(bloom);

    bloom = sg_scaling_bloom_filter_alloc_from_file(CAPACITY, ERROR_RATE, BLOOM_FILE);

    for (size_t i = 0; i < CAPACITY; i++) {
        char word[256];
        sprintf(word, "%zu", i);
        int key_removed = (i % 5 == 0);
        EXPECT_FALSE(!key_removed && !sg_scaling_bloom_filter_contain(bloom, word, strlen(word)))
            << "false negative occurred";
    }

    sg_scaling_bloom_filter_free(bloom);
}
