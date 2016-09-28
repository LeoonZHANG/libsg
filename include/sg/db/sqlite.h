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


/* connection operation */

sg_sqlite_conn_t *sg_sqlite_open(const char *filename, const char *charset, const char *usr, const char *pwd, bool create_if_not_exist);

void sg_sqlite_close(sg_sqlite_conn_t *conn);



/* database and table operation */

bool sg_sqlite_create_db(sg_sqlite_conn_t *conn, const char *db_name);

bool sg_sqlite_create_tab(sg_sqlite_conn_t *conn, const char *db_name, const char *tab_name);

bool sg_sqlite_list_db(sg_sqlite_conn_t *conn, sg_vsstr_list_t *out);

bool sg_sqlite_list_tab(sg_sqlite_conn_t *conn, const char *db_name, sg_vsstr_list_t *out);

bool sg_sqlite_remove_db(sg_sqlite_conn_t *conn, const char *db_name);

bool sg_sqlite_remove_tab(sg_sqlite_conn_t *conn, const char *db_name, const char *tab_name);

bool sg_sqlite_exec(sg_sqlite_conn_t *conn, const char *sql, sg_sqlite_exec_cb_t cb, void *ctx);





/* table record operation */

sg_sqlite_tab_t *sg_sqlite_tab_open(sg_sqlite_conn_t *conn, const char *db_name, const char *tab_name);

bool sg_sqlite_tab_list(sg_sqlite_tab_t *tab, int limit, int skip, sg_sqlite_exec_cb_t cb, void *ctx);

void sg_sqlite_tab_close(sg_sqlite_tab_t *tab);




/* record parse operation */

int sg_sqlite_rec_size(sg_sqlite_rec_t *rec);

bool sg_sqlite_rec_parse(sg_sqlite_rec_t *rec, int idx, const char *attr, void *out_val, enum sg_sqlite_val_type out_type);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SQLLITE_H */
