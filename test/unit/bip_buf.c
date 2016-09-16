#include <sg/container/bip_buf.h>

int main(void)
{
    sg_bip_buf_t *bf;
    int is_empty;
    unsigned char *buf;
    char tmp[256] = "abcd";

    bf = sg_bip_buf_create(100);
    assert(bf);

    is_empty = sg_bip_buf_is_empty(bf);
    assert(is_empty == 1);

    buf = sg_bip_buf_get(bf, 1);
    assert(buf == NULL);

    for (int i = 0; i < 100; i++) {
        sg_bip_buf_put(bf, tmp, 4);
    }
}