#include <sg/crypto/mac.h>
#include <gtest/gtest.h>

static struct test_st {
    unsigned char key[16];
    int key_len;
    unsigned char data[64];
    int data_len;
    const char *digest;
} test[8] = {
    {
        "", 0, "More text test vectors to stuff up EBCDIC machines :-)", 54,
        "e9139d1e6ee064ef8cf514fc7dc83e86",
    },
    {
        "Jefe", 4, "what do ya want for nothing?", 28,
        "750c783e6ab0b503eaa86e310a5db738",
    },
    {
        "", 0, "My test data", 12,
        "61afdecb95429ef494d61fdee15990cabf0826fc"
    },
    {
        "12345", 5, "My test data again", 18,
        "a12396ceddd2a85f4c656bc1e0aa50c78cffde3e"
    },
    {
        "", 0, "My test data", 12,
        "2274b195d90ce8e03406f4b526a47e0787a88a65479938f1a5baa3ce0f079776"
    },
    {
        "123456", 6, "My test data", 12,
        "bab53058ae861a7f191abe2d0145cbb123776a6369ee3f9d79ce455667e411dd"
    }
};

static void test_data(struct test_st* data, enum sg_mac_type type)
{
    struct sg_mac_sum rst;
    sg_mac_str((const char*)data->data, type, (const char*)data->key, &rst);
    EXPECT_STREQ(rst.hex_str, data->digest);
}

TEST(test_crypto_mac, md5_without_key)
{
    test_data(&test[0], SGMACTYPE_HMAC_MD5);
}

TEST(test_crypto_mac, md5_with_key)
{
    test_data(&test[1], SGMACTYPE_HMAC_MD5);
}

TEST(test_crypto_mac, sha1_without_key)
{
    test_data(&test[2], SGMACTYPE_HMAC_SHA1);
}

TEST(test_crypto_mac, sha1_with_key)
{
    test_data(&test[3], SGMACTYPE_HMAC_SHA1);
}

TEST(test_crypto_mac, sha256_without_key)
{
    test_data(&test[4], SGMACTYPE_HMAC_SHA256);
}

TEST(test_crypto_mac, sha256_with_key)
{
    test_data(&test[5], SGMACTYPE_HMAC_SHA256);
}
