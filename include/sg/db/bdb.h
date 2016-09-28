/**
 * dbd.h
 * Easy-to-use berkeley db APIs.
 * key和value的类型是否需要调整?
 */

#ifndef LIBSG_BDB_H
#define LIBSG_BDB_H

#include <sg/sg.h>
#include <sg/container/vsbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_bdb_conn sg_bdb_conn_t;

typedef struct sg_bdb_coll sg_bdb_coll_t;


/* val will be free after callback is done */
typedef void (*sg_ejdb_coll_list_cb_t)(const void *key, const void *val, void *ctx);




/* connection operation */
sg_bdb_conn_t *sg_bdb_open(const char *filename, const char *usr, const char *pwd,
        const char *charset, uint32_t cache_size, bool create_if_not_exist);

bool sg_bdb_close(sg_bdb_conn_t *conn);





/* database and collection operation */

bool sg_bdb_create_db(sg_bdb_conn_t *conn, const char *db_name);

bool sg_bdb_create_coll(sg_bdb_conn_t *conn, const char *db_name, const char *coll_name);

bool sg_bdb_list_db(sg_bdb_conn_t *conn, sg_vsstr_list_t *out);

bool sg_bdb_list_coll(sg_bdb_conn_t *conn, const char *db_name, sg_vsstr_list_t *out);

bool sg_bdb_remove_db(sg_bdb_conn_t *conn, const char *db_name);

bool sg_bdb_remove_coll(sg_bdb_conn_t *conn, const char *db_name, const char *coll_name);






/* KV operation */
sg_bdb_coll_t *sg_bdb_coll_open(sg_bdb_conn_t *conn, const char *db_name, const char *coll_name);

bool sg_bdb_coll_put(sg_bdb_coll_t *coll, void *key, void *val, uint64_t val_size);

bool sg_bdb_coll_get(sg_bdb_coll_t *coll, void *key, sg_vsbuf_t *out_val);

bool sg_bdb_coll_remove(sg_bdb_coll_t *coll, void *key);

bool sg_bdb_coll_list(sg_bdb_coll_t *coll, int limit, int skip, sg_ejdb_coll_list_cb_t db, void *ctx);

bool sg_bdb_coll_update(sg_bdb_coll_t *coll, void *key, void *val, uint64_t val_size);

bool sg_bdb_coll_close(sg_bdb_coll_t *coll);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BDB_H */
