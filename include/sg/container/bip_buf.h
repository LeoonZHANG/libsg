/*
 * bip_buf.h
 * Bip buffer data structure.
 */

#ifndef LIBSG_BIP_BUF_H
#define LIBSG_BIP_BUF_H

typedef struct sg_bip_buf sg_bip_buf_t;

/**
 * Create a new bip buffer.
 *
 * malloc()s space
 *
 * @param[in] size The size of the buffer
 * @param[in] auto_inc Auto increment if put data size is bigger than init size
 */
sg_bip_buf_t *sg_bip_buf_create(size_t size, bool auto_inc);

/**
 * Look at data. Don't move cursor
 *
 * @param[in] len The length of the data to be peeked
 * @return data on success, NULL if we can't peek at this much data */
unsigned char *sg_bip_buf_peek(const sg_bip_buf_t *me, const unsigned int len);

/* ? */
/**
 * Get pointer to data to read. Move the cursor on.
 *
 * @param[in] len The length of the data to be polled
 * @return pointer to data, NULL if we can't poll this much data */
unsigned char *sg_bip_buf_get(sg_bip_buf_t *me, const unsigned int size);

/**
 * @param[in] data The data to be offered to the buffer
 * @param[in] size The size of the data to be offered
 * @return number of bytes offered, return -1 if error such like put size is too big */
int sg_bip_buf_put(sg_bip_buf_t *me, const unsigned char *data, const int size);

/**
 * @return how much space we have assigned */
int sg_bip_buf_used_size(const sg_bip_buf_t *cb);

/**
 * @return bytes of unused space */
int sg_bip_buf_unused_size(const sg_bip_buf_t *me);

/**
 * @return the size of the bipbuffer */
int sg_bip_buf_size(const sg_bip_buf_t *me);

/**
 * @return 1 if buffer is empty; 0 otherwise */
int sg_bip_buf_is_empty(const sg_bip_buf_t *me);

/**
 * Free the bip buffer */
void sg_bip_buf_destroy(sg_bip_buf_t *me);

#endif /* LIBSG_BIP_BUF_H */