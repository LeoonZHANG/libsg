#include <sg/crypto/md.h>
#include <gtest/gtest.h>

const char *text[] = {
	"",
	"a",
	"abc",
	"message digest",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"12345678901234567890123456789012345678901234567890123456789012345678901234567890"
};

TEST(test_crypto_md, md2)
{
    const char *result[] = {
        "8350e5a3e24c153df2275c9f80692773",
        "32ec01ec4a6dac72c0ab96fb34c0b5d1",
        "da853b0d3f88d99b30283a69e6ded6bb",
        "ab4f496bfb2a530b219ff33031fe06b0",
        "4e8ddff3650292ab5a4108c3aa47940b",
        "da33def2a42df13975352846c30338cd",
        "d5976f79d83d3a0dc9806c3c66f3efd8",
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_MD2, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, md4)
{
    const char *result[] = {
        "31d6cfe0d16ae931b73c59d7e0c089c0",
        "bde52cb31de33e46245e05fbdbd6fb24",
        "a448017aaf21d8525fc10ae87aa6729d",
        "d9130a8164549fe818874806e1c7014b",
        "d79e1c308aa5bbcdeea8ed63df412da9",
        "043f8582f241db351ce627e153e7f0e4",
        "e33b4ddc9c38f2199c3e7b164fcc0536",
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_MD4, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, md5)
{
    const char *result[] = {
        "d41d8cd98f00b204e9800998ecf8427e",
        "0cc175b9c0f1b6a831c399e269772661",
        "900150983cd24fb0d6963f7d28e17f72",
        "f96b697d7cb7938d525a2f31aaf161d0",
        "c3fcd3d76192e4007dfb496cca67e13b",
        "d174ab98d277d9f5a5611c2c9f419d9f",
        "57edf4a22be3c955ac49da2e2107b67a"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_MD5, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, sha1)
{
    const char *result[] = {
		"da39a3ee5e6b4b0d3255bfef95601890afd80709",
        "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8",
        "a9993e364706816aba3e25717850c26c9cd0d89d",
        "c12252ceda8be8994d5fa0290a47231c1d16aae3",
        "32d10c7b8cf96570ca04ce37f2a19d84240d3a89",
        "761c457bf73b14d27e9e9265c46f4b4dda11f940",
        "50abf5706a150990a08b2c5ea40fa0e585554732",
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_SHA1, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, sha224)
{
    const char *result[] = {
		"d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
		"abd37534c7d9a2efb9465de931cd7055ffdb8879563ae98078d6d6d5",
		"23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
		"2cb21c83ae2f004de7e81c3c7019cbcb65b71ab656b22d6d0c39b8eb",
		"45a5f72c39c5cff2522eb3429799e49e5f44b356ef926bcf390dccc2",
		"bff72b4fcb7d75e5632900ac5f90d219e05e97a7bde72e740db393d9",
		"b50aecbe4e9bb0b57bc5f3ae760a8e01db24f203fb3cdcd13148046e"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_SHA224, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, sha256)
{
    const char *result[] = {
		"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
		"ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb",
		"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
		"f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650",
		"71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73",
		"db4bfcbd4da0cd85a60c3c37d3fbd8805c77f15fc6b1fdfe614ee0a7c8fdb4c0",
		"f371bc4a311f2b009eef952dd83ca80e2b60026c8e935592d0f9c308453c813e"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_SHA256, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, sha384)
{
    const char *result[] = {
		"38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
		"54a59b9f22b0b80880d8427e548b7c23abd873486e1f035dce9cd697e85175033caa88e6d57bc35efae0b5afd3145f31",
		"cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7",
		"473ed35167ec1f5d8e550368a3db39be54639f828868e9454c239fc8b52e3c61dbd0d8b4de1390c256dcbb5d5fd99cd5",
		"feb67349df3db6f5924815d6c3dc133f091809213731fe5c7b5f4999e463479ff2877f5f2936fa63bb43784b12f3ebb4",
		"1761336e3f7cbfe51deb137f026f89e01a448e3b1fafa64039c1464ee8732f11a5341a6f41e0c202294736ed64db1a84",
		"b12932b0627d1c060942f5447764155655bd4da0c9afa6dd9b9ef53129af1b8fb0195996d2de9ca0df9d821ffee67026"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_SHA384, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, sha512)
{
    const char *result[] = {
		"cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
		"1f40fc92da241694750979ee6cf582f2d5d7d28e18335de05abc54d0560e0f5302860c652bf08d560252aa5e74210546f369fbbbce8c12cfc7957b2652fe9a75",
		"ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f",
		"107dbf389d9e9f71a3a95f6c055b9251bc5268c2be16d6c13492ea45b0199f3309e16455ab1e96118e8a905d5597b72038ddb372a89826046de66687bb420e7c",
		"4dbff86cc2ca1bae1e16468a05cb9881c97f1753bce3619034898faa1aabe429955a1bf8ec483d7421fe3c1646613a59ed5441fb0f321389f77f48a879c7b1f1",
		"1e07be23c26a86ea37ea810c8ec7809352515a970e9253c26f536cfc7a9996c45c8370583e0a78fa4a90041d71a4ceab7423f19c71b9d5a3e01249f0bebd5894",
		"72ec1ef1124a45b047e8b7c75a932195135bb61de24ec0d1914042246e0aec3a2354e093d76f3048b456764346900cb130d2a4fd5dd16abb5e30bcb850dee843"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_SHA512, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}

TEST(test_crypto_md, ripemd)
{
    const char *result[] = {
		"9c1185a5c5e9fc54612808977ee8f548b2258d31",
		"0bdc9d2d256b3ee9daae347be6f4dc835a467ffe",
		"8eb208f7e05d987a9b044a8e98c6b087f15a0bfc",
		"5d0689ef49d2fae572b881b123a85ffa21595f36",
		"f71c27109c692c1b56bbdceb5b9d2865b3708dbc",
		"b0e20b6e3116640286ed3a87a5713079b21f5189",
		"9b752e45573d4b39f4dbd3323cab82bf63326bfb"
    };

    for (size_t i = 0; i < sizeof(text) / sizeof(text[0]); ++i) {
        struct sg_md_sum digest;
        sg_md_str(text[i], SGMDTYPE_RIPEMD, &digest);
        EXPECT_STREQ(digest.hex_str, result[i])
            << " text #" << i << ": " << text[i];
    }
}
