/**
 * ejdb.h
 * Easy-to-use ejdb(embbed json database) APIs.
 */

#ifndef LIBSG_EJDB_H
#define LIBSG_EJDB_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct sg_ejdb sg_ejdb_t;

typedef struct sg_ejdb_db  sg_ejdb_db_t;

typedef struct sg_ejdb_tab sg_ejdb_tab_t;

typedef struct sg_ejdb_rec sg_ejdb_rec_t;

sg_ejdb_t *sg_ejdb_open(const char *filename, const char *usr, const char *pwd);

sg_ejdb_db_t *sg_ejdb_open_db(sg_ejdb_t *ins);

bool sg_ejdb_exec(sg_ejdb_t *self, const char *javascript);

void sg_ejdb_close(sg_ejdb_t *self);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_EJDB_H */
