/**
 * sqlite.h
 * Sqlite3 access library with simple APIs.
 */

#ifndef LIBSG_SQLLITE_H
#define LIBSG_SQLLITE_H

#include <sg/sg.h>
#include "../str/vsstr_list.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct sg_sqlite_conn sg_sqlite_conn_t;

typedef struct sg_sqlite_db  sg_sqlite_db_t;

typedef struct sg_sqlite_tab sg_sqlite_tab_t;

typedef struct sg_sqlite_rec sg_sqlite_rec_t;

/* rec will be free after callback is done */
typedef void (*sg_sqlite_list_cb_t)(sg_sqlite_rec_t *rec, void *ctx);

/* rec will be free after callback is done */
typedef void (*sg_sqlite_exec_cb_t)(sg_sqlite_rec_t *rec, void *ctx);


enum sg_sqlite_val_type {
    SGSQLITEVALTYPE_INT   = 0, /* int32_t */
    SGSQLITEVALTYPE_FLOAT = 1, /* float */
    SGSQLITEVALTYPE_STR   = 2, /* sg_vsstr_t */
    SGSQLITEVALTYPE_BLOB  = 3  /* sg_vsbuf_t */
};



/* if filename doesn't not existing, create a new sqlite db file */
sg_sqlite_conn_t *sg_sqlite_open_conn(const char *filename, const char *charset, const char *usr, const char *pwd);

sg_sqlite_db_t *sg_sqlite_open_db(sg_sqlite_conn_t *conn, const char *name);

sg_sqlite_tab_t *sg_sqlite_open_tab(sg_sqlite_db_t *db, const char *name);




bool sg_sqlite_create_db(sg_sqlite_conn_t *conn, const char *name);





bool sg_sqlite_list_db(sg_sqlite_conn_t *conn, sg_vsstr_list_t *out);

bool sg_sqlite_list_tab(sg_sqlite_db_t *db, sg_vsstr_list_t *out);

bool sg_sqlite_list_rec(sg_sqlite_db_t *db, int limit, int skip, sg_sqlite_exec_cb_t cb, void *ctx);




bool sg_sqlite_exec(sg_sqlite_conn_t *conn, const char *sql, sg_sqlite_exec_cb_t cb, void *ctx);

int sg_sqlite_rec_size(sg_sqlite_rec_t *rec);

bool sg_sqlite_rec_parse(sg_sqlite_rec_t *rec, int idx, const char *attr, void *out_val, enum sg_sqlite_val_type out_type);




bool sg_sqlite_remove_db(sg_sqlite_conn_t *conn, const char *name);

bool sg_sqlite_remove_tab(sg_sqlite_db_t *db, const char *name);




void sg_sqlite_close_tab(sg_sqlite_tab_t *tab);

void sg_sqlite_close_db(sg_sqlite_db_t *db);

void sg_sqlite_close_conn(sg_sqlite_conn_t *conn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SQLLITE_H */
