/*
 * xml.h
 * Xpath style XML library based on fastest xml library in world ~ vtd-xml.
 */

#ifndef LIBSG_XML_H
#define LIBSG_XML_H

#include <stddef.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* type of JSON value */
enum xml_val_type {
    XMLVALTYPE_ERROR,	/* error */
    XMLVALTYPE_NULL,	/* null */
    XMLVALTYPE_BOOL,	/* boolean value (true, false) */
    XMLVALTYPE_NUMBER,	/* numeric */
    XMLVALTYPE_STRING,	/* literal string */
    XMLVALTYPE_ARRAY,	/* array */
    XMLVALTYPE_OBJECT   /* object */
};

typedef void xml_doc_t;

typedef char xml_str_t;


void xml_init(void);

void xml_free(void);

xml_doc_t *xml_alloc_doc(const char *path);

xml_doc_t *xml_alloc_doc2(const char *buffer, size_t size);

xml_doc_t *xml_alloc_doc3(void);

/* 不要忘记使用xml_free_string释放xml_str_t */
int xml_doc_to_str(xml_doc_t *, xml_str_t **);

int xml_get_member_size(xml_doc_t *, const char *xpath);

enum xml_val_type xml_get_type(xml_doc_t *, const char *xpath);

int xml_get_int(xml_doc_t *, const char *xpath, int *);

int xml_get_double(xml_doc_t *, const char *xpath, double *);

int xml_get_bool(xml_doc_t *, const char *xpath, bool *);

/* 不要忘记使用xml_free_string释放xml_str_t */
int xml_get_string(xml_doc_t *, const char *xpath, xml_str_t **);

int xml_set_int(xml_doc_t *, const char *xpath, int);

int xml_set_double(xml_doc_t *, const char *xpath, double);

int xml_set_bool(xml_doc_t *, const char *xpath, bool);

int xml_set_string(xml_doc_t *, const char *xpath, const char *);

void xml_free_string(xml_str_t *);

void xml_free_doc(xml_doc_t *);


//=============== vtd-xml =================

//=============== libxml ===================


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_XML_H */
