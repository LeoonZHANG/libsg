#include <stdio.h>
#include "../../include/json.h"

int main(void)
{
    sg_json_doc_t *d;
    sg_json_pointer_t *p;
    char *res;
    size_t len;
    sg_vlstr *str;

    d = sg_json_alloc_doc("/home/metalwood/json.json");
    p = sg_json_alloc_pointer("/FBI/0/name");
    if (sg_json_get_string(d, p, &res, &len) != 0)
        return -1;
    str = sg_vlstrndup(res, len);
    printf("result:%s.\n", sg_vlstrraw(str));
    sg_vlstrfree(&str);
    sg_json_set_string(d, p, "new title");
    sg_json_doc_to_file(d, "/home/metalwood/json2.json");

    return 0;
}

