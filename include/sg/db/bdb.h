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

/*
enum sg_bdb_charset {
    SG
};*/

typedef struct sg_bdb_ins  sg_bdb_ins_t;

typedef struct sg_bdb_db   sg_bdb_db_t;

typedef struct sg_bdb_coll sg_bdb_coll_t;

typedef struct sg_bdb_key  sg_bdb_key_t;

typedef struct sg_bdb_val  sg_bdb_val_t;




sg_bdb_ins_t *sg_bdb_open(const char *filename, const char *usr, const char *pwd,
        const char *charset, uint32_t cache_size);

bool sg_bdb_list_db(sg_bdb_ins_t *ins);

bool sg_bdb_list_coll(sg_bdb_db_t *db)

sg_bdb_db_t *sg_bdb_open_db(sg_bdb_ins_t *ins, const char *db_name);

sg_bdb_coll_t *sg_bdb_open_coll(sg_bdb_db_t *db, const char *coll_name);

bool sg_bdb_put(sg_bdb_coll_t *coll, void *key, void *val);

bool sg_bdb_get

bool sg_bdb_del

uint64_t sg_bdb_val_size



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BDB_H */
