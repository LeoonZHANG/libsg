/*
 * zip.h
 *
 */

#ifndef ZIP_H_
#define ZIP_H_

int do_zip(char *in, int in_size, char **dest, int *dest_size);

int do_unzip(char *in, int in_size, char **dest, int *dest_size);

#endif /* ZIP_H_ */
