#include <string.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/ripemd.h>
#include <sg/sg.h>
#include <sg/crypto/md.h>

/* Message digest handle opened by sg_md_start. */
struct sg_md_ctx {
    enum sg_md_type type;
    union {
        MD2_CTX md2;
        MD4_CTX md4;
        MD5_CTX md5;
        SHA_CTX sha1;
        SHA256_CTX sha224;
        SHA256_CTX sha256;
        SHA512_CTX sha384;
        SHA512_CTX sha512;
        RIPEMD160_CTX ripemd;
    };
};

/* Open a message digest context. */
sg_md_t *sg_md_start(enum sg_md_type type)
{
    sg_md_t* result = calloc(1, sizeof(sg_md_t));
    result->type = type;
    switch (type) {
    case SGMDTYPE_MD2:
        MD2_Init(&result->md2);
		break;
    case SGMDTYPE_MD4:
        MD4_Init(&result->md4);
		break;
    case SGMDTYPE_MD5:
        MD5_Init(&result->md5);
		break;
    case SGMDTYPE_SHA1:
        SHA1_Init(&result->sha1);
		break;
    case SGMDTYPE_SHA224:
        SHA224_Init(&result->sha224);
		break;
    case SGMDTYPE_SHA256:
        SHA256_Init(&result->sha256);
		break;
    case SGMDTYPE_SHA384:
        SHA384_Init(&result->sha384);
		break;
    case SGMDTYPE_SHA512:
        SHA512_Init(&result->sha512);
		break;
    case SGMDTYPE_RIPEMD:
        RIPEMD160_Init(&result->ripemd);
		break;
	default:
		free(result);
		return NULL;
    }
	return result;
}

/* Update data to hash it.
   Length must > 0. 填入数据大小不限制,如果算法接口限制了每次接口调用时传入数据的大小,请在此模块内部解决这个差异。 */
int sg_md_update(sg_md_t *ctx, const void *buf, size_t len)
{
    switch (ctx->type) {
    case SGMDTYPE_MD2:
        return MD2_Update(&ctx->md2, buf, len);
    case SGMDTYPE_MD4:
        return MD4_Update(&ctx->md4, buf, len);
    case SGMDTYPE_MD5:
        return MD5_Update(&ctx->md5, buf, len);
    case SGMDTYPE_SHA1:
        return SHA1_Update(&ctx->sha1, buf, len);
    case SGMDTYPE_SHA224:
        return SHA224_Update(&ctx->sha224, buf, len);
    case SGMDTYPE_SHA256:
        return SHA256_Update(&ctx->sha256, buf, len);
    case SGMDTYPE_SHA384:
        return SHA384_Update(&ctx->sha384, buf, len);
    case SGMDTYPE_SHA512:
        return SHA512_Update(&ctx->sha512, buf, len);
    case SGMDTYPE_RIPEMD:
		return RIPEMD160_Update(&ctx->ripemd, buf, len);
    }
}

/* Close the message digest context and get the result. */
int sg_md_finish(sg_md_t *md, struct sg_md_sum *rst)
{
    switch (md->type) {
    case SGMDTYPE_MD2:
        MD2_Final(rst->raw_bin, &md->md2);
        rst->raw_bin_used_len = 16;
        break;
    case SGMDTYPE_MD4:
        MD4_Final(rst->raw_bin, &md->md4);
        rst->raw_bin_used_len = 16;
        break;
    case SGMDTYPE_MD5:
        MD5_Final(rst->raw_bin, &md->md5);
        rst->raw_bin_used_len = 16;
        break;
    case SGMDTYPE_SHA1:
        SHA1_Final(rst->raw_bin, &md->sha1);
        rst->raw_bin_used_len = SHA_DIGEST_LENGTH;
        break;
    case SGMDTYPE_SHA224:
        SHA224_Final(rst->raw_bin, &md->sha224);
        rst->raw_bin_used_len = SHA224_DIGEST_LENGTH;
        break;
    case SGMDTYPE_SHA256:
        SHA256_Final(rst->raw_bin, &md->sha256);
        rst->raw_bin_used_len = SHA256_DIGEST_LENGTH;
        break;
    case SGMDTYPE_SHA384:
        SHA384_Final(rst->raw_bin, &md->sha384);
        rst->raw_bin_used_len = SHA384_DIGEST_LENGTH;
        break;
    case SGMDTYPE_SHA512:
        SHA512_Final(rst->raw_bin, &md->sha512);
        rst->raw_bin_used_len = SHA512_DIGEST_LENGTH;
        break;
    case SGMDTYPE_RIPEMD:
		RIPEMD160_Final(rst->raw_bin, &md->ripemd);
        rst->raw_bin_used_len = RIPEMD160_DIGEST_LENGTH;
        break;
    }
    free(md);
    for (size_t i = 0; i < rst->raw_bin_used_len; ++i)
        sprintf(rst->hex_str + i * 2, "%02x", rst->raw_bin[i]);
    return 0;
}

/* Message digest hash for a binary buffer.
   Length must > 0. */
int sg_md_buf(const void *buf, size_t len, enum sg_md_type type, struct sg_md_sum *rst)
{
    sg_md_t * md = sg_md_start(type);
    sg_md_update(md, buf, len);
    sg_md_finish(md, rst);
    return 0;
}

/* Message digest hash for a c plain string.
   String length must > 0. */
int sg_md_str(const char *str, enum sg_md_type type, struct sg_md_sum *rst)
{
    return sg_md_buf(str, strlen(str), type, rst);
}

/* Message digest hash for a file.
   Length of path must > 0. */
int sg_md_file(const char *path, enum sg_md_type type, struct sg_md_sum *rst)
{
    sg_md_t * md = sg_md_start(type);

    FILE *file = fopen(path, "rb");
    unsigned char data[1024];
    int bytes;
    while ((bytes = fread(data, sizeof(data[0]), sizeof(data), file)) != 0)
        sg_md_update(md, data, bytes);
    fclose(file);

    sg_md_finish(md, rst);
    return 0;
}
