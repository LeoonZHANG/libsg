/**
 * ejdb.h
 * Easy-to-use ejdb(embbed json database) APIs.
 * ejdb本身可能提供了json文档的内置类型,请转换成sg_json_doc_t,这个模块要方便, 效率不重要
 */

#ifndef LIBSG_EJDB_H
#define LIBSG_EJDB_H

#include <sg/sg.h>
#include <sg/container/json.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct sg_ejdb_conn  sg_ejdb_conn_t;

typedef struct sg_ejdb_db   sg_ejdb_db_t;

typedef struct sg_ejdb_coll sg_ejdb_coll_t;

typedef struct sg_ejdb_doc  sg_ejdb_doc_t;

/* doc will be free after callback is done */
typedef void (*sg_ejdb_list_docs_cb_t)(const sg_json_doc_t *doc, void *ctx);

/* doc will be free after callback is done */
typedef void (*sg_ejdb_exec_cb_t)(const sg_json_doc_t *rec, void *ctx);


/* if filename doesn't not existing, create a new ejdb db file */
sg_ejdb_conn_t *sg_ejdb_open_conn(const char *filename, const char *usr, const char *pwd);

sg_ejdb_db_t *sg_ejdb_open_db(sg_ejdb_conn_t *conn, const char *name);

sg_ejdb_coll_t *sg_ejdb_open_coll(sg_ejdb_db_t *db, const char *name);



bool sg_ejdb_create_db(sg_ejdb_conn_t *conn, const char *name);

bool sg_ejdb_create_coll(sg_ejdb_db_t *db, const char *name);




bool sg_ejdb_list_db(sg_ejdb_conn_t *conn, sg_vsstr_list_t *out);

bool sg_ejdb_list_coll(sg_ejdb_db_t *db, sg_vsstr_list_t *out);

bool sg_ejdb_list_docs(sg_ejdb_coll_t *coll, int limit, int skip, sg_ejdb_list_docs_cb_t db, void *ctx);




bool sg_ejdb_exec(sg_ejdb_db_t *self, const char *query, sg_ejdb_exec_cb_t cb, void *ctx);

bool sg_ejdb_insert_doc(sg_ejdb_t *coll, const sg_json_doc_t *doc);

/* alloc doc_out(sg_json_doc_t) before using it */
bool sg_ejdb_find_doc(sg_ejdb_t *coll, const char *doc_id, sg_json_doc_t *doc_out);

bool sg_ejdb_update_doc(sg_ejdb_t *coll, const char *doc_id, const sg_json_doc_t *doc);




bool sg_ejdb_remove_doc(sg_ejdb_t *coll, const char *doc_id);

bool sg_ejdb_remove_coll(sg_ejdb_db_t *db, const char *coll_name);

bool sg_ejdb_remove_db(sg_ejdb_conn_t *conn, const char *db_name);



void sg_ejdb_close_conn(sg_ejdb_conn_t *conn);

void sg_ejdb_close_db(sg_ejdb_db_t *db);

void sg_ejdb_close_coll(sg_ejdb_coll_t *coll);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_EJDB_H */
