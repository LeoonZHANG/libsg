#include <sg/math/speed.h>
#include <sg/sys/clock.h>
#include <gtest/gtest.h>

class test_math_speed : public ::testing::Test
{
protected:
    sg_speed_val* _speed;

    virtual void SetUp()
    {
        _speed = sg_speed_alloc();
    }

    virtual void TearDown()
    {
        sg_speed_free(_speed);
    }
};

TEST_F(test_math_speed, set)
{
    sg_speed_set(_speed, (1 << 20), SGSPEEDUNIT_bps);
    ASSERT_EQ(_speed->val, 1048576);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_bps);

    sg_speed_conv(_speed, SGSPEEDUNIT_Kbps);
    ASSERT_EQ(_speed->val, 1024);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_Kbps);

    sg_speed_conv_auto(_speed);
    ASSERT_EQ(_speed->val, 1);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_Mbps);

    sg_speed_set(_speed, 1, SGSPEEDUNIT_Gbps);
    sg_speed_conv(_speed, SGSPEEDUNIT_bps);
    ASSERT_EQ(_speed->val, 1073741824);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_bps);

    sg_speed_conv(_speed, SGSPEEDUNIT_Bps);
    ASSERT_EQ(_speed->val, 1073741824/8);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_Bps);

    sg_speed_set(_speed, 1, SGSPEEDUNIT_Tbps);
    sg_speed_conv(_speed, SGSPEEDUNIT_bps);
    ASSERT_EQ(_speed->val, 1073741824 * 1024.0);
    ASSERT_EQ(_speed->unit, SGSPEEDUNIT_bps);
}

class test_math_speed_counter : public ::testing::Test
{
protected:
    sg_speed_counter_t* _counter;
    sg_speed_val* _speed;

    virtual void SetUp()
    {
        _counter = sg_speed_counter_open(100);
        _speed = sg_speed_alloc();
    }

    virtual void TearDown()
    {
        sg_speed_counter_close(_counter);
        sg_speed_free(_speed);
    }
};

static void sleep_ms(size_t ms)
{
    size_t tick = sg_clock_boot_time_ms();
    size_t tock = sg_clock_boot_time_ms();
    while (tock < tick + ms)
        tock = sg_clock_boot_time_ms();
    tick = tock;
}

TEST_F(test_math_speed_counter, statistic)
{
    for (size_t i = 0; i < 100; ++i)
        sg_speed_counter_reg(_counter, 100 * (i + 1));
    sg_speed_counter_read(_counter, _speed);
    EXPECT_EQ(_speed->val, 493.1640625);
    /* slots: 505000 */
    EXPECT_EQ(_speed->unit, SGSPEEDUNIT_KBps);

    for (size_t c = 0; c < 3; ++c) {
        sleep_ms(20);
        for (size_t i = 0; i < 100; ++i)
            sg_speed_counter_reg(_counter, 100 * (i + 1));
    }
    sg_speed_counter_read(_counter, _speed);
    /* slots: 505000 | 505000 | 505000 | 505000 */
    EXPECT_EQ(_speed->val, 1.926422119140625);
    EXPECT_EQ(_speed->unit, SGSPEEDUNIT_MBps);

    for (size_t c = 0; c < 3; ++c) {
        sleep_ms(20);
        for (size_t i = 0; i < 100; ++i)
            sg_speed_counter_reg(_counter, 200 * (i + 1));
    }
    sg_speed_counter_read(_counter, _speed);
    /* slots: ~~505000~~ | ~~505000~~ | 505000 | 505000 | 1010000
                1010000  | 1010000 */
    EXPECT_EQ(_speed->val, 3.85284423828125);
    EXPECT_EQ(_speed->unit, SGSPEEDUNIT_MBps);
}
