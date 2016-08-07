#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include "json.h"


/* as shown above, each token can be either string or number, and there must be a '/' before each token */
int main()
{
    struct sg_json_doc jdoc;
    struct sg_json_value jvalue;
    int out=0;
    double dout;
    const char *sout, *jsonstr;
    char jpath[50];
    bool bout;
    const char *token = "/";
    const char *json_string = "{\"monitor\": {\"ip\":\"172.20.220.156\", \"port\":8088,\"double\":8.88,\"group\":{\"group_name\":\"cntv\", \"author\":true,"
            "\"group_data\":[\"wheel\", \"video\", \"user\"]}}}";

    if (sg_json_parse(&jdoc, json_string))
        return -1;

    /* Get a int-type value according to jpath */
    strncpy(jpath, "/monitor/port", 50);
    sg_json_get_val_int(&jdoc, jpath, &out, token);
    printf("--INT:%d\n",out);
    /* Get a double-type value according to jpath */
    strncpy(jpath, "/monitor/double", 50);
    sg_json_get_val_double(&jdoc, jpath, &dout, token);
    printf("--DOUBLE:%f\n",dout);
    /* Get a string-type value according to jpath */
    strncpy(jpath, "/monitor/group/group_data/0", 50);
    sg_json_get_val_string(&jdoc, jpath, &sout, token);
    printf("--STRING:%s\n",sout);
    /* Get a array size according to jpath */
    strncpy(jpath, "/monitor/group/group_data/?", 50);
    sg_json_get_array_size(&jdoc, jpath, &out, token);
    printf("--SIZE: %d\n",out);
    /* Get a bool-type size according to jpath */
    strncpy(jpath, "/monitor/group/author", 50);
    sg_json_get_val_bool(&jdoc, jpath, &bout, token);
    printf("--BOOL :%s\n", bout? "true":"false");

    /* Encode JSON document and get printable string. */
    const char *link[] = {"http:/fhefh/fheo.mp4","http:/fhefh/34126578.mp4"};
    struct sg_json_media_parma parameter;
    struct sg_json_array_parma array_parm;
    parameter.web_name = "Twitter";
    parameter.account = "xmnews";
    parameter.publishtime = "2015-12-15 12:00:00";
    parameter.crawltime = "2015-12-15 12:00:00";
    parameter.url = "http://fweij/dweyoao.html";
    parameter.content = "hello,world!";
    parameter.like = 45;
    parameter.share = 456;
    parameter.comment = 78;
    parameter.isforward = false;
    parameter.is_encodejson_done = 0;
    array_parm.web_name = "Twitter";
    array_parm.array_name = "VideoLink";
    array_parm.is_encodejson_done = 1;

    sg_json_add_members(&jdoc, parameter, &jvalue);
    sg_json_add_array(&jdoc, array_parm, link,2, &jvalue);
    sg_json_convert_string (&jdoc, &jsonstr);
	printf(" JSONSTR:%s\n",jsonstr);
    return 0;
}