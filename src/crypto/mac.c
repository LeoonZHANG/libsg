#include <string.h>
#include <stdio.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <sg/sg.h>
#include <sg/crypto/mac.h>

struct sg_mac {
    HMAC_CTX *pimpl;
};

static const EVP_MD* get_evp_md(enum sg_mac_type type)
{
    switch (type) {
    case SGMACTYPE_HMAC_MD2:
        return EVP_md2();
    case SGMACTYPE_HMAC_MD4:
        return EVP_md4();
    case SGMACTYPE_HMAC_MD5:
        return EVP_md5();
    case SGMACTYPE_HMAC_SHA1:
        return EVP_sha1();
    case SGMACTYPE_HMAC_SHA224:
        return EVP_sha224();
    case SGMACTYPE_HMAC_SHA256:
        return EVP_sha256();
    case SGMACTYPE_HMAC_SHA384:
        return EVP_sha384();
    case SGMACTYPE_HMAC_SHA512:
        return EVP_sha512();
    default:
        return NULL;
    }
}

/* Mac hash for a binary buffer.
   Length must > 0. */
int sg_mac_buf(const void *buf, size_t len, enum sg_mac_type type, const char *key, struct sg_mac_sum *rst)
{
    sg_mac_ctx* mac = sg_mac_start(type, key);
    sg_mac_update(mac, buf, len);
    sg_mac_finish(mac, rst);
    return 0;
}

/* Mac for a c plain string.
   String length must > 0. */
int sg_mac_str(const char *str, enum sg_mac_type type, const char *key, struct sg_mac_sum *rst)
{
    return sg_mac_buf(str, strlen(str), type, key, rst);
}

/* Mac hash for a file.
   Length of path must > 0. */
int sg_mac_file(const char *path, enum sg_mac_type type, const char *key, struct sg_mac_sum *rst)
{
    sg_mac_ctx* mac = sg_mac_start(type, key);

    FILE *file = fopen(path, "rb");
    if (file)
        return -1;

    unsigned char data[1024];
    int bytes;
    while ((bytes = fread(data, sizeof(data[0]), sizeof(data), file)) != 0)
        sg_mac_update(mac, data, bytes);
    fclose(file);

    sg_mac_finish(mac, rst);
    return 0;
}

/* Open a mac context. */
sg_mac_ctx *sg_mac_start(enum sg_mac_type type, const char *key)
{
    sg_mac_ctx* result = calloc(1, sizeof(sg_mac_ctx));
    result->pimpl = HMAC_CTX_new();
    HMAC_Init_ex(result->pimpl, key, strlen(key), get_evp_md(type), NULL);
    return result;
}

/* Update data to hash it.
   Length must > 0. */
int sg_mac_update(sg_mac_ctx *mac, const void *input, size_t input_len)
{
    return HMAC_Update(mac->pimpl, input, input_len);
}

/* Close the mac context and get the result. */
int sg_mac_finish(sg_mac_ctx *mac, struct sg_mac_sum *rst)
{
    if (!mac)
        return -1;

    HMAC_Final(mac->pimpl, rst->raw_bin, &rst->raw_bin_used_len);
    HMAC_CTX_free(mac->pimpl);
    free(mac);

    for (size_t i = 0; i < rst->raw_bin_used_len; ++i)
        sprintf(rst->hex_str + i * 2, "%02x", rst->raw_bin[i]);
    return 0;
}

