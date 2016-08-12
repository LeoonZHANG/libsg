/*
 * zip.h
 *
 */

#ifndef LIBSG_ZIP_H
#define LIBSG_ZIP_H

int sg_zip_compress(char *in, int in_size, char **dest, int *dest_size);

int sg_zip_uncompress(char *in, int in_size, char **dest, int *dest_size);

#endif /* LIBSG_ZIP_H */
