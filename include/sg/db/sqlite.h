/**
 * sqlite.h
 * Author: wangwei.
 * Sqlite access library with simple APIs.
 */

#ifndef LIBSG_SQLLITE_H
#define LIBSG_SQLLITE_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct sg_sqlite sg_sqlite_t;

typedef struct sg_sqlite_db  sg_sqlite_db_t;

typedef struct sg_sqlite_tab sg_sqlite_tab_t;

typedef struct sg_sqlite_rec sg_sqlite_rec_t;

sg_sqlite_t *sg_sqlite_open(const char *filename, const char *usr, const char *pwd);

sg_sqlite_db_t *sg_sqlite_open_db(sg_sqlite_t *ins);

bool sg_sqlite_exec(sg_sqlite_t *self, const char *sql);

void sg_sqlite_close(sg_sqlite_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SQLLITE_H */
