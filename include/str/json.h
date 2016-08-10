/*
 * json.h
 * JSON pointer style JSON wrapper based on cJson.
 */

#ifndef LIB_JSON_H
#define LIB_JSON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* type of JSON value */
enum json_val_type {
    JSONVALTYPE_ERROR,	/* error */
    JSONVALTYPE_NULL,	/* null */
    JSONVALTYPE_BOOL,	/* boolean value (true, false) */
    JSONVALTYPE_NUMBER,	/* numeric */
    JSONVALTYPE_STRING,	/* literal string */
    JSONVALTYPE_ARRAY,	/* array */
    JSONVALTYPE_OBJECT  /* object */
};

typedef void json_doc_t;

typedef char json_str_t;

json_doc_t *json_alloc_doc(const char *filename);

json_doc_t *json_alloc_doc2(const void *buf, size_t size);

json_doc_t *json_alloc_doc3(void);

/* 不要忘记使用json_free_string释放json_str_t */
int json_doc_to_str(json_doc_t *, json_str_t **);

int json_doc_to_file(json_doc_t *, const char *filename);

int json_get_member_size(json_doc_t *, const char *json_pointer,int *size);

enum json_val_type json_get_type(json_doc_t *, const char *json_pointer);

int json_get_int(json_doc_t *, const char *json_pointer, int *);

int json_get_double(json_doc_t *, const char *json_pointer, double *);

int json_get_bool(json_doc_t *, const char *json_pointer, bool *);

/* 不要忘记使用json_free_string释放json_str_t */
int json_get_string(json_doc_t *, const char *json_pointer, json_str_t **);

/*不需要释放返回的json_doc_t* */
int json_get_object(json_doc_t *,const char *json_pointer,json_doc_t **value);
/**/
int json_set_object(json_doc_t *,const char *json_pointer,json_doc_t *value);

int json_set_int(json_doc_t *, const char *json_pointer, int);

int json_set_double(json_doc_t *, const char *json_pointer, double);

int json_set_bool(json_doc_t *, const char *json_pointer, bool);

int json_set_string(json_doc_t *, const char *json_pointer, const char *);

void json_free_string(json_str_t *);

void json_free_doc(json_doc_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBJSON_H */
