#include <assert.h>
#include <sg/container/bip_buf.h>

int main(void)
{
    sg_bip_buf_t *bf;
    unsigned char put_buf[4] = "abcd";
    unsigned char *get_buf = NULL;

    bf = sg_bip_buf_create(400);
    assert(bf);
    assert(sg_bip_buf_max_payload_size(bf) == 400);
    assert(sg_bip_buf_is_empty(bf) == 1);
    assert(sg_bip_buf_get(bf, 1) == NULL);

    for (int i = 0; i < 100; i++) {
        assert(sg_bip_buf_put(bf, put_buf, 4) == 4);
        assert(sg_bip_buf_used_size(bf) == (i + 1) * 4);
        assert(sg_bip_buf_unused_size(bf) == (400 - ((i + 1) * 4)));
        assert(sg_bip_buf_is_empty(bf) == 0);
    }

    assert(sg_bip_buf_put(bf, put_buf, 4) == 0);

    for (int i = 0; i < 100; i++) {
        assert(sg_bip_buf_is_empty(bf) == 0);
        get_buf = sg_bip_buf_get(bf, 4);
        assert(get_buf);
        assert(strstr(get_buf, "abcd") == get_buf);
        assert(sg_bip_buf_used_size(bf) == (400 - ((i + 1) * 4)));
        assert(sg_bip_buf_unused_size(bf) == (i + 1) * 4);

    }

    assert(sg_bip_buf_get(bf, 4) == NULL);
    assert(sg_bip_buf_is_empty(bf) == 1);

    printf("test pass\n");
    return 0;
}