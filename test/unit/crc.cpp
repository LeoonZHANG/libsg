#include <sg/hash/crc.h>
#include <gtest/gtest.h>

TEST(test_hash_crc, crc8)
{
    sg_crc_sum crc = {0};
    EXPECT_EQ(0, sg_crc_str("abcdefg", SGCRCTYPE_CRC_8, &crc));
    EXPECT_EQ(178, crc.val_crc_8);
}

TEST(test_hash_crc, crc16)
{
    sg_crc_sum crc = {0};
    EXPECT_EQ(0, sg_crc_str("abcdefg", SGCRCTYPE_CRC_16, &crc));
    EXPECT_EQ(59865, crc.val_crc_16);
}

TEST(test_hash_crc, crc32)
{
    sg_crc_sum crc = {0};
    EXPECT_EQ(0, sg_crc_str("abcdefg", SGCRCTYPE_CRC_32, &crc));
    EXPECT_EQ(2890315224, crc.val_crc_32);
}

TEST(test_hash_crc, crc64)
{
    sg_crc_sum crc = {0};
    EXPECT_EQ(0, sg_crc_str("abcdefg", SGCRCTYPE_CRC_64, &crc));
    EXPECT_EQ(3715203883376380826UL, crc.val_crc_64);
}

TEST(test_hash_crc, crc_ccitt)
{
    sg_crc_sum crc = {0};
    EXPECT_EQ(0, sg_crc_str("abcdefg", SGCRCTYPE_CRC_CCITT, &crc));
    EXPECT_EQ(63756, crc.val_crc_ccitt);
}
