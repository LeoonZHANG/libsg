/**
 * dbd.h
 * Easy-to-use berkeley db APIs.
 */

#ifndef LIBSG_BDB_H
#define LIBSG_BDB_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_bdb_conn  sg_bdb_conn_t;

typedef struct sg_bdb_db   sg_bdb_db_t;

typedef struct sg_bdb_coll sg_bdb_coll_t;

typedef void               sg_bdb_val_t;

/* val will be free after callback is done */
typedef void (*sg_bdb_list_val_cb_t)(const sg_bdb_val_t *val, void *ctx);



/* if filename doesn't not existing, create a new bdb db file */
sg_bdb_conn_t *sg_bdb_open(const char *filename, const char *usr, const char *pwd,
        const char *charset, uint32_t cache_size);

sg_bdb_db_t *sg_bdb_open_db(sg_bdb_conn_t *conn, const char *name);

sg_bdb_coll_t *sg_bdb_open_coll(sg_bdb_db_t *db, const char *name);



bool sg_bdb_list_db(sg_bdb_conn_t *conn, sg_vsstr_list_t *out);

bool sg_bdb_list_coll(sg_bdb_db_t *db, sg_vsstr_list_t *out);

bool sg_bdb_list_val(sg_bdb_coll_t *coll, int limit, int skip, sg_bdb_list_val_cb_t cb, void *ctx);



bool sg_bdb_put(sg_bdb_coll_t *coll, void *key, void *val);

void *sg_bdb_get(sg_bdb_coll_t *coll, void *key);



bool sg_bdb_remove_db(sg_bdb_conn_t *conn, const char *name);

bool sg_bdb_remove_coll(sg_bdb_db_t *db, const char *name);

bool sg_bdb_remove_val(sg_bdb_coll_t *coll, const char *key);



bool sg_bdb_close_conn(sg_bdb_conn_t *conn);

bool sg_bdb_close_db(sg_bdb_db_t *db);

bool sg_bdb_close_coll(sg_bdb_coll_t *coll);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BDB_H */
