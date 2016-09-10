#include <gtest/gtest.h>
#include <sg/crypto/sym.h>

/* A 256 bit key */
const char* key = "01234567890123456789012345678901";

/* A 128 bit IV */
const char* iv = "01234567890123456";

/* Message to be encrypted */
const char* plaintext = "The quick brown fox jumps over the lazy dog";

static void cipher_callback(const void* data, size_t size, void* user_data)
{
    sg_vlbuf_t* out = (sg_vlbuf_t*)user_data;
    sg_vlbuf_insert(out, data, size);
}

TEST(test_crypto_sym, callback)
{
    sg_vlbuf_t* cipher = sg_vlbuf_create();
    sg_vlbuf_t* decipher = sg_vlbuf_create();

    sg_sym_t* encrypter = sg_sym_alloc(SGSYMTYPE_AES_CBC, SGSYMMODE_ENC, cipher_callback);
    sg_sym_set_key(encrypter, (const unsigned char*)key, strlen(key), (const unsigned char*)iv, iv ? strlen(iv) : 0);
    sg_sym_set_user_data(encrypter, cipher);
    sg_sym_put(encrypter, plaintext, strlen(plaintext));
    sg_sym_flush(encrypter);
    sg_sym_free(encrypter);

    sg_sym_t* decrypter = sg_sym_alloc(SGSYMTYPE_AES_CBC, SGSYMMODE_DEC, cipher_callback);
    sg_sym_set_key(decrypter, (const unsigned char*)key, strlen(key), (const unsigned char*)iv, iv ? strlen(iv) : 0);
    sg_sym_set_user_data(decrypter, decipher);
    sg_sym_put(decrypter, cipher->mem, cipher->size);
    sg_sym_flush(decrypter);
    sg_sym_free(decrypter);

    EXPECT_EQ(decipher->size, strlen(plaintext));
    EXPECT_EQ(0, memcmp(decipher->mem, plaintext, strlen(plaintext)));

    sg_vlbuf_destroy(cipher);
    sg_vlbuf_destroy(decipher);
}

static void test_crypto(enum sg_sym_type type)
{
    sg_vlbuf_t* cipher = sg_vlbuf_create();
    sg_vlbuf_t* decipher = sg_vlbuf_create();

    sg_sym_str(type, key, iv, SGSYMMODE_ENC, plaintext, cipher);
    sg_sym_buf(type, key, iv, SGSYMMODE_DEC, cipher->mem, cipher->size, decipher);

    EXPECT_EQ(decipher->size, strlen(plaintext));
    EXPECT_EQ(0, memcmp(decipher->mem, plaintext, strlen(plaintext)));

    sg_vlbuf_destroy(cipher);
    sg_vlbuf_destroy(decipher);
}

TEST(test_crypto_sym, aes_256_cbc) { test_crypto(SGSYMTYPE_AES_CBC); }
TEST(test_crypto_sym, aes_256_ecb) { test_crypto(SGSYMTYPE_AES_ECB); }
TEST(test_crypto_sym, aes_256_cfb128) { test_crypto(SGSYMTYPE_AES_CFB128); }
TEST(test_crypto_sym, aes_256_cfb1) { test_crypto(SGSYMTYPE_AES_CFB1); }
TEST(test_crypto_sym, aes_256_cfb8) { test_crypto(SGSYMTYPE_AES_CFB8); }
TEST(test_crypto_sym, aes_256_ofb) { test_crypto(SGSYMTYPE_AES_OFB); }

TEST(test_crypto_sym, rc2_cbc) { test_crypto(SGSYMTYPE_RC2_CBC); }
TEST(test_crypto_sym, rc2_cfb) { test_crypto(SGSYMTYPE_RC2_CFB); }
TEST(test_crypto_sym, rc2_ecb) { test_crypto(SGSYMTYPE_RC2_ECB); }
TEST(test_crypto_sym, rc2_ofb) { test_crypto(SGSYMTYPE_RC2_OFB); }

TEST(test_crypto_sym, rc4) { test_crypto(SGSYMTYPE_RC4); }
TEST(test_crypto_sym, rc4_40) { test_crypto(SGSYMTYPE_RC4_40); }
TEST(test_crypto_sym, rc4_hmac_md5) { test_crypto(SGSYMTYPE_RC4_HMAC_MD5); }

TEST(test_crypto_sym, rc5_32_12_16_cbc) { test_crypto(SGSYMTYPE_RC5_32_12_16_CBC); }
TEST(test_crypto_sym, rc5_32_12_16_ecb) { test_crypto(SGSYMTYPE_RC5_32_12_16_ECB); }
TEST(test_crypto_sym, rc5_32_12_16_cfb64) { test_crypto(SGSYMTYPE_RC5_32_12_16_CFB64); }
TEST(test_crypto_sym, rc5_32_12_16_ofb) { test_crypto(SGSYMTYPE_RC5_32_12_16_OFB); }

TEST(test_crypto_sym, idea_ecb) { test_crypto(SGSYMTYPE_IDEA_ECB); }
TEST(test_crypto_sym, idea_cfb64) { test_crypto(SGSYMTYPE_IDEA_CFB64); }
TEST(test_crypto_sym, idea_ofb) { test_crypto(SGSYMTYPE_IDEA_OFB); }
TEST(test_crypto_sym, idea_cbc) { test_crypto(SGSYMTYPE_IDEA_CBC); }

TEST(test_crypto_sym, bf_ecb) { test_crypto(SGSYMTYPE_BF_ECB); }
TEST(test_crypto_sym, bf_cfb64) { test_crypto(SGSYMTYPE_BF_CFB64); }
TEST(test_crypto_sym, bf_ofb) { test_crypto(SGSYMTYPE_BF_OFB); }
TEST(test_crypto_sym, bf_cbc) { test_crypto(SGSYMTYPE_BF_CBC); }

TEST(test_crypto_sym, des_ecb) { test_crypto(SGSYMTYPE_DES_ECB); }
TEST(test_crypto_sym, des_ede) { test_crypto(SGSYMTYPE_DES_EDE); }
TEST(test_crypto_sym, des_ede3) { test_crypto(SGSYMTYPE_DES_EDE3); }
TEST(test_crypto_sym, des_ede_ecb) { test_crypto(SGSYMTYPE_DES_EDE_ECB); }
TEST(test_crypto_sym, des_ede3_ecb) { test_crypto(SGSYMTYPE_DES_EDE3_ECB); }
TEST(test_crypto_sym, des_cfb64) { test_crypto(SGSYMTYPE_DES_CFB64); }
TEST(test_crypto_sym, des_cfb1) { test_crypto(SGSYMTYPE_DES_CFB1); }
TEST(test_crypto_sym, des_cfb8) { test_crypto(SGSYMTYPE_DES_CFB8); }
TEST(test_crypto_sym, des_ede_cfb64) { test_crypto(SGSYMTYPE_DES_EDE_CFB64); }
TEST(test_crypto_sym, des_ede3_cfb64) { test_crypto(SGSYMTYPE_DES_EDE3_CFB64); }
TEST(test_crypto_sym, des_ede3_cfb1) { test_crypto(SGSYMTYPE_DES_EDE3_CFB1); }
TEST(test_crypto_sym, des_ede3_cfb8) { test_crypto(SGSYMTYPE_DES_EDE3_CFB8); }
TEST(test_crypto_sym, des_ofb) { test_crypto(SGSYMTYPE_DES_OFB); }
TEST(test_crypto_sym, des_ede_ofb) { test_crypto(SGSYMTYPE_DES_EDE_OFB); }
TEST(test_crypto_sym, des_ede3_ofb) { test_crypto(SGSYMTYPE_DES_EDE3_OFB); }
TEST(test_crypto_sym, des_cbc) { test_crypto(SGSYMTYPE_DES_CBC); }
TEST(test_crypto_sym, des_ede_cbc) { test_crypto(SGSYMTYPE_DES_EDE_CBC); }
TEST(test_crypto_sym, des_ede3_cbc) { test_crypto(SGSYMTYPE_DES_EDE3_CBC); }
