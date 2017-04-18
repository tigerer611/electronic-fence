#ifndef __CIRC_BUF_H
#define __CIRC_BUF_H
#include <stdint.h>

struct circ_buf {
        uint8_t *buf;
        int head;
        int tail;
        uint32_t size;
};

void circ_init1(struct circ_buf* circ, uint8_t *buf, uint32_t size);
void circ_init2(struct circ_buf* circ, uint8_t *buf, uint32_t size);
void circ_init3(struct circ_buf* circ, uint8_t *buf, uint32_t size);
int circ_put1(struct circ_buf* circ, uint8_t data);
int circ_put2(struct circ_buf* circ, uint8_t data);
int circ_put3(struct circ_buf* circ, uint8_t data);
int circ_get1(struct circ_buf* circ, uint8_t *data);
int circ_get2(struct circ_buf* circ, uint8_t *data);
int circ_get3(struct circ_buf* circ, uint8_t *data);

/* Return count in buffer.  */
#define CIRC_CNT(head, tail, size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define CIRC_SPACE(head, tail, size) CIRC_CNT((tail), ((head)+1), (size))

#endif
