/**
 * xml.h
 * Xpath style XML library based on fastest xml library in world ~ vtd-xml.
 */

#ifndef LIBSG_XML_H
#define LIBSG_XML_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* type of JSON value */
enum sg_xml_val_type {
    SGXMLVALTYPE_ERROR,	/* error */
    SGXMLVALTYPE_NULL,	/* null */
    SGXMLVALTYPE_BOOL,	/* boolean value (true, false) */
    SGXMLVALTYPE_NUMBER,	/* numeric */
    SGXMLVALTYPE_STRING,	/* literal string */
    SGXMLVALTYPE_ARRAY,	/* array */
    SGXMLVALTYPE_OBJECT   /* object */
};

typedef void sg_xml_doc_t;

typedef char sg_xml_str_t;


void sg_xml_init(void);

void sg_xml_free(void);

sg_xml_doc_t *sg_xml_alloc_doc(const char *path);

sg_xml_doc_t *sg_xml_alloc_doc2(const char *buffer, size_t size);

sg_xml_doc_t *sg_xml_alloc_doc3(void);

/* remeber to free sg_xml_str_t with sg_xml_free_string */
int sg_xml_doc_to_str(sg_xml_doc_t *, sg_xml_str_t **);

int sg_xml_get_member_size(sg_xml_doc_t *, const char *xpath);

enum sg_xml_val_type sg_xml_get_type(sg_xml_doc_t *, const char *xpath);

int sg_xml_get_int(sg_xml_doc_t *, const char *xpath, int *);

int sg_xml_get_double(sg_xml_doc_t *, const char *xpath, double *);

int sg_xml_get_bool(sg_xml_doc_t *, const char *xpath, bool *);

/* remeber to free sg_xml_str_t with sg_xml_free_string */
int sg_xml_get_string(sg_xml_doc_t *, const char *xpath, sg_xml_str_t **);

int sg_xml_set_int(sg_xml_doc_t *, const char *xpath, int);

int sg_xml_set_double(sg_xml_doc_t *, const char *xpath, double);

int sg_xml_set_bool(sg_xml_doc_t *, const char *xpath, bool);

int sg_xml_set_string(sg_xml_doc_t *, const char *xpath, const char *);

void sg_xml_free_string(sg_xml_str_t *);

void sg_xml_free_doc(sg_xml_doc_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_XML_H */
