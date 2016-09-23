#include <string.h>
#include <stdio.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <sg/crypto/sym.h>

#undef min
#undef max
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define DEFAULT_OUTPUT_BUFFER_SIZE 512

struct sg_sym_real {
    enum sg_sym_type type;
    enum sg_sym_mode mode;
    sg_sym_on_data_func_t on_data;
    void* user_data;
    unsigned char* output;
    size_t output_size;
    EVP_CIPHER_CTX* pimpl;
    const EVP_CIPHER* cipher;
};

static int the_openssl_crypto_initialized = 0;

static void ensure_openssl_crypto_initialized()
{
    if (the_openssl_crypto_initialized)
        return;

    /* Initialise the library */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    the_openssl_crypto_initialized = 1;
}

// FIXME: cleanup won't be called
static void openssl_crypto_cleanup()
{
    if (!the_openssl_crypto_initialized)
        return;

    /* Clean up */
    EVP_cleanup();
    ERR_free_strings();
    the_openssl_crypto_initialized = 0;
}

static const EVP_CIPHER* get_cipher_by_type(enum sg_sym_type type)
{
    switch (type) {
    case SGSYMTYPE_AES_CBC:
        return EVP_aes_256_cbc();
    case SGSYMTYPE_AES_CFB128:
        return EVP_aes_256_cfb128();
    case SGSYMTYPE_AES_CFB1:
        return EVP_aes_256_cfb1();
    case SGSYMTYPE_AES_CFB8:
        return EVP_aes_256_cfb8();
    case SGSYMTYPE_AES_ECB:
        return EVP_aes_256_ecb();
    case SGSYMTYPE_AES_OFB:
        return EVP_aes_256_ofb();
    case SGSYMTYPE_RC2_ECB:
        return EVP_rc2_ecb();
    case SGSYMTYPE_RC2_CBC:
        return EVP_rc2_cbc();
    case SGSYMTYPE_RC2_CFB:
        return EVP_rc2_cfb();
    case SGSYMTYPE_RC2_OFB:
        return EVP_rc2_ofb();
    case SGSYMTYPE_RC4:
        return EVP_rc4();
    case SGSYMTYPE_RC4_40:
        return EVP_rc4_40();
    case SGSYMTYPE_RC4_HMAC_MD5:
        return EVP_rc4_hmac_md5();
    case SGSYMTYPE_RC5_32_12_16_CBC:
        return EVP_rc5_32_12_16_cbc();
    case SGSYMTYPE_RC5_32_12_16_ECB:
        return EVP_rc5_32_12_16_ecb();
    case SGSYMTYPE_RC5_32_12_16_CFB64:
        return EVP_rc5_32_12_16_cfb64();
    case SGSYMTYPE_RC5_32_12_16_OFB:
        return EVP_rc5_32_12_16_ofb();
    case SGSYMTYPE_IDEA_ECB:
        return EVP_idea_ecb();
    case SGSYMTYPE_IDEA_CFB64:
        return EVP_idea_cfb64();
    case SGSYMTYPE_IDEA_OFB:
        return EVP_idea_ofb();
    case SGSYMTYPE_IDEA_CBC:
        return EVP_idea_cbc();
    case SGSYMTYPE_BF_ECB:
        return EVP_bf_ecb();
    case SGSYMTYPE_BF_CFB64:
        return EVP_bf_cfb64();
    case SGSYMTYPE_BF_OFB:
        return EVP_bf_ofb();
    case SGSYMTYPE_BF_CBC:
        return EVP_bf_cbc();
    case SGSYMTYPE_DES_ECB:
        return EVP_des_ecb();
    case SGSYMTYPE_DES_EDE:
        return EVP_des_ede();
    case SGSYMTYPE_DES_EDE3:
        return EVP_des_ede3();
    case SGSYMTYPE_DES_EDE_ECB:
        return EVP_des_ede_ecb();
    case SGSYMTYPE_DES_EDE3_ECB:
        return EVP_des_ede3_ecb();
    case SGSYMTYPE_DES_CFB64:
        return EVP_des_cfb64();
    case SGSYMTYPE_DES_CFB1:
        return EVP_des_cfb1();
    case SGSYMTYPE_DES_CFB8:
        return EVP_des_cfb8();
    case SGSYMTYPE_DES_EDE_CFB64:
        return EVP_des_ede_cfb64();
    case SGSYMTYPE_DES_EDE3_CFB64:
        return EVP_des_ede3_cfb64();
    case SGSYMTYPE_DES_EDE3_CFB1:
        return EVP_des_ede3_cfb1();
    case SGSYMTYPE_DES_EDE3_CFB8:
        return EVP_des_ede3_cfb8();
    case SGSYMTYPE_DES_OFB:
        return EVP_des_ofb();
    case SGSYMTYPE_DES_EDE_OFB:
        return EVP_des_ede_ofb();
    case SGSYMTYPE_DES_EDE3_OFB:
        return EVP_des_ede3_ofb();
    case SGSYMTYPE_DES_CBC:
        return EVP_des_cbc();
    case SGSYMTYPE_DES_EDE_CBC:
        return EVP_des_ede_cbc();
    case SGSYMTYPE_DES_EDE3_CBC:
        return EVP_des_ede3_cbc();
    default:
        return NULL;
    }
}

sg_sym_t *sg_sym_alloc(enum sg_sym_type type, enum sg_sym_mode mode, sg_sym_on_data_func_t cb)
{
    if (mode != SGSYMMODE_ENC && mode != SGSYMMODE_DEC)
        return NULL;

    ensure_openssl_crypto_initialized();

    const EVP_CIPHER* cipher = get_cipher_by_type(type);
    if (!cipher)
        return NULL;

    EVP_CIPHER_CTX* pimpl = EVP_CIPHER_CTX_new();
    if (!pimpl)
        return NULL;

    sg_sym_t* result = calloc(1, sizeof(sg_sym_t));
    result->type = type;
    result->mode = mode;
    result->on_data = cb;
    result->pimpl = pimpl;
    result->cipher = cipher;

    return result;
}

void sg_sym_free(sg_sym_t *sym)
{
    if (sym) {
        if (sym->pimpl)
            EVP_CIPHER_CTX_free(sym->pimpl);
        if (sym->output)
            free(sym->output);
        free(sym);
    }
}

void sg_sym_set_user_data(sg_sym_t *sym, void *user_data)
{
    sym->user_data = user_data;
}

int sg_sym_set_key(sg_sym_t *sym, const unsigned char *key, size_t numbits, const unsigned char* iv, size_t iv_numbits)
{
    if (sym->mode == SGSYMMODE_ENC)
        return EVP_EncryptInit_ex(sym->pimpl, sym->cipher, NULL, key, iv);
    else
        return EVP_DecryptInit_ex(sym->pimpl, sym->cipher, NULL, key, iv);
}

static void callback(sg_sym_t* sym)
{
    if (sym->output_size)
        sym->on_data(sym->output, sym->output_size, sym->user_data);
    sym->output_size = 0;
}

void sg_sym_put(sg_sym_t *sym, const void *data, size_t size)
{
    int transform_size = 0;
    sym->output = calloc(size, 1);
    if (sym->mode == SGSYMMODE_ENC)
        EVP_EncryptUpdate(sym->pimpl, sym->output, &transform_size, data, size);
    else
        EVP_DecryptUpdate(sym->pimpl, sym->output, &transform_size, data, size);

    sym->output_size = transform_size;
    callback(sym);
    free(sym->output);
    sym->output = NULL;
}

void sg_sym_flush(sg_sym_t* sym)
{
    int transform_size;
    sym->output = calloc(DEFAULT_OUTPUT_BUFFER_SIZE, 1);
    if (sym->mode == SGSYMMODE_ENC)
        EVP_EncryptFinal_ex(sym->pimpl, sym->output, &transform_size);
    else
        EVP_DecryptFinal_ex(sym->pimpl, sym->output, &transform_size);

    sym->output_size = transform_size;
    callback(sym);
    free(sym->output);
    sym->output = NULL;
}

static void buffer_output_callback(const void* data, size_t size, void *user_data)
{
    sg_vsbuf_t* res = (sg_vsbuf_t*) user_data;
    if (res)
        sg_vsbuf_insert(res, data, size);
}

int sg_sym_buf(enum sg_sym_type type, const char *key, const char *iv, enum sg_sym_mode mode, const void *src, size_t src_len, sg_vsbuf_t *res)
{
    sg_sym_t* sym = sg_sym_alloc(type, mode, buffer_output_callback);
    sg_sym_set_key(sym, (const unsigned char*) key, strlen(key), (const unsigned char*) iv, iv ? strlen(iv) : 0);
    sg_sym_set_user_data(sym, res);
    sg_sym_put(sym, src, src_len);
    sg_sym_flush(sym);
    sg_sym_free(sym);
    return 0;
}

int sg_sym_str(enum sg_sym_type type, const char *key, const char *iv, enum sg_sym_mode mode, const char *src, sg_vsbuf_t *res)
{
    return sg_sym_buf(type, key, iv, mode, src, strlen(src), res);
}

static void file_output_callback(const void* data, size_t size, void *user_data)
{
    FILE* fo = (FILE*) user_data;
    fwrite(data, 1, size, fo);
}

int sg_sym_file(enum sg_sym_type type, const char *key, const char *iv, enum sg_sym_mode mode, const char *src_filename, const char *res_filename)
{
    sg_sym_t* sym = sg_sym_alloc(type, mode, file_output_callback);
    sg_sym_set_key(sym, (const unsigned char*) key, strlen(key), (const unsigned char*) iv, iv ? strlen(iv) : 0);

    FILE *fo = fopen(res_filename, "a+b");
    sg_sym_set_user_data(sym, fo);

    FILE *fi = fopen(src_filename, "rb");
    unsigned char data[1024];
    int bytes;
    while ((bytes = fread(data, sizeof(data[0]), sizeof(data), fi)) != 0)
        sg_sym_put(sym, data, bytes);
    fclose(fi);

    sg_sym_flush(sym);

    fclose(fo);

    sg_sym_free(sym);
    return 0;
}

