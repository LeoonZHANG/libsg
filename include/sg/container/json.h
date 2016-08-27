/*
 * json.h
 * JSON pointer style JSON wrapper based on cJson.
 */

#ifndef LIBSG_JSON_H
#define LIBSG_JSON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* type of JSON value */
enum sg_json_val_type {
    SGJSONVALTYPE_ERROR,	/* error */
    SGJSONVALTYPE_NULL,	/* null */
    SGJSONVALTYPE_BOOL,	/* boolean value (true, false) */
    SGJSONVALTYPE_NUMBER,	/* numeric */
    SGJSONVALTYPE_STRING,	/* literal string */
    SGJSONVALTYPE_ARRAY,	/* array */
    SGJSONVALTYPE_OBJECT 	/* object */
};

typedef void sg_json_doc_t;

typedef char sg_json_str_t;

/*
 * Check json is valid or not.
 * If valid, it returns 0.
 */
int sg_json_check(const char *json, int check_depth);

sg_json_doc_t *sg_json_alloc_doc(const char *filename);

sg_json_doc_t *sg_json_alloc_doc2(const void *buf, size_t size);

sg_json_doc_t *sg_json_alloc_doc3(void);

/* Don’t forget to free sg_json_str_t with sg_json_free_string. */
int sg_json_doc_to_str(sg_json_doc_t *, sg_json_str_t **);

int sg_json_doc_to_file(sg_json_doc_t *, const char *filename);

int sg_json_get_member_size(sg_json_doc_t *, const char *sg_json_pointer, int *size);

enum sg_json_val_type sg_json_get_type(sg_json_doc_t *, const char *sg_json_pointer);

int sg_json_get_int(sg_json_doc_t *, const char *sg_json_pointer, int *);

int sg_json_get_double(sg_json_doc_t *, const char *sg_json_pointer, double *);

int sg_json_get_bool(sg_json_doc_t *, const char *sg_json_pointer, bool *);

/* Don’t forget to free sg_json_str_t with sg_json_free_string. */
int sg_json_get_string(sg_json_doc_t *, const char *sg_json_pointer, sg_json_str_t **);

/* Don’t free the return value sg_json_doc_t */
int sg_json_get_object(sg_json_doc_t *, const char *sg_json_pointer, sg_json_doc_t **value);
/**/
int sg_json_set_object(sg_json_doc_t *, const char *sg_json_pointer, sg_json_doc_t *value);

int sg_json_set_int(sg_json_doc_t *, const char *sg_json_pointer, int);

int sg_json_set_double(sg_json_doc_t *, const char *sg_json_pointer, double);

int sg_json_set_bool(sg_json_doc_t *, const char *sg_json_pointer, bool);

int sg_json_set_string(sg_json_doc_t *, const char *sg_json_pointer, const char *);

void sg_json_free_string(sg_json_str_t *);

void sg_json_free_doc(sg_json_doc_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_JSON_H */
