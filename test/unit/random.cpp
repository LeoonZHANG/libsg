#include <sg/math/random.h>
#include <gtest/gtest.h>

TEST(test_math_random, generate)
{
    for (size_t i = 0; i < 10000; ++i)
        ASSERT_NE(0, sg_random_l());
}

TEST(test_math_random, generate_float)
{
    for (size_t i = 0; i < 10000; ++i)
        EXPECT_PRED_FORMAT2(testing::FloatLE, 0, sg_random_f());
}

TEST(test_math_random, generate_range)
{
    for (size_t i = 0; i < 10000; ++i) {
        long value = sg_random_range_l(100, 102);
        ASSERT_TRUE(100 <= value && value < 102);
    }
    for (size_t i = 0; i < 10000; ++i) {
        long value = sg_random_range_l(144300, 144392);
        ASSERT_TRUE(144300 <= value && value < 144392);
    }
}

TEST(test_math_random, generate_range_float)
{
    for (size_t i = 0; i < 10000; ++i) {
        float value = sg_random_range_f(0.45, 0.46);
        EXPECT_PRED_FORMAT2(testing::FloatLE, 0.45, value);
        EXPECT_PRED_FORMAT2(testing::FloatLE, value, 0.46);
    }
}

