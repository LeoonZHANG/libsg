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

typedef struct sg_ejdb_coll sg_ejdb_coll_t;

/* doc will be free after callback is done */
typedef void (*sg_ejdb_coll_list_cb_t)(const char *doc_id, const char *json, void *ctx);

/* doc will be free after callback is done */
typedef void (*sg_ejdb_exec_cb_t)(const char *json, void *ctx);






sg_ejdb_conn_t *sg_ejdb_open(const char *filename, const char *usr, const char *pwd, bool create_if_not_exist);

void sg_ejdb_close(sg_ejdb_conn_t *conn);






/* database and collection operation */

bool sg_ejdb_create_db(sg_ejdb_conn_t *conn, const char *db_name);

bool sg_ejdb_create_coll(sg_ejdb_conn_t *conn, const char *db_name, const char *coll_name);

bool sg_ejdb_list_db(sg_ejdb_conn_t *conn, sg_vsstr_list_t *out);

bool sg_ejdb_list_coll(sg_ejdb_conn_t *conn, const char *db_name, sg_vsstr_list_t *out);

bool sg_ejdb_remove_db(sg_ejdb_conn_t *conn, const char *db_name);

bool sg_ejdb_remove_coll(sg_ejdb_conn_t *conn, const char *db_name, const char *coll_name);

bool sg_ejdb_exec(sg_ejdb_conn_t *conn, const char *query, sg_ejdb_exec_cb_t cb, void *ctx);



/* doc operation */

sg_ejdb_coll_t *sg_ejdb_coll_open(sg_ejdb_conn_t *conn, const char *db_name, const char *coll_name);

bool sg_ejdb_coll_put(sg_ejdb_coll_t *coll, const char *json);

bool sg_ejdb_coll_get(sg_ejdb_coll_t *coll, const char *doc_id, sg_vsstr_t *out_json);

bool sg_ejdb_coll_remove(sg_ejdb_coll_t *coll, const char *doc_id);

bool sg_ejdb_coll_list(sg_ejdb_coll_t *coll, int limit, int skip, sg_ejdb_coll_list_cb_t db, void *ctx);

bool sg_ejdb_coll_update(sg_ejdb_coll_t *coll, const char *doc_id, const char *json);

void sg_ejdb_coll_close(sg_ejdb_coll_t *coll);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_EJDB_H */
