#include "circ_buf.h"

// size is 2^n
void circ_init1(struct circ_buf* circ, uint8_t *buf, uint32_t size)
{
  circ->buf = buf;
  circ->head = circ->tail = 0;
  circ->size = size;
}

void circ_init2(struct circ_buf* circ, uint8_t *buf, uint32_t size)
{
  circ->buf = buf;
  circ->head = circ->tail = 0;
  circ->size = size;
}

void circ_init3(struct circ_buf* circ, uint8_t *buf, uint32_t size)
{
  circ->buf = buf;
  circ->head = circ->tail = 0;
  circ->size = size;
}

// size is 2^n
int circ_put1(struct circ_buf* circ, uint8_t data)
{
  if(CIRC_SPACE(circ->head, circ->tail, circ->size) == 0)
    return -1;

  circ->buf[circ->head] = data;
  circ->head = (circ->head + 1) & (circ->size - 1);
  return 0;
}

int circ_put2(struct circ_buf* circ, uint8_t data)
{
  if(CIRC_SPACE(circ->head, circ->tail, circ->size) == 0)
    return -1;

  circ->buf[circ->head] = data;
  circ->head = (circ->head + 1) & (circ->size - 1);
  return 0;
}

int circ_put3(struct circ_buf* circ, uint8_t data)
{
  if(CIRC_SPACE(circ->head, circ->tail, circ->size) == 0)
    return -1;

  circ->buf[circ->head] = data;
  circ->head = (circ->head + 1) & (circ->size - 1);
  return 0;
}

// size is 2^n, say important things three times
int circ_get1(struct circ_buf* circ, uint8_t *data)
{
  if (CIRC_CNT(circ->head, circ->tail, circ->size) == 0)
    return -1;

  *data = circ->buf[circ->tail];
  circ->tail = (circ->tail + 1) & (circ->size - 1);
  return 0;
}

int circ_get2(struct circ_buf* circ, uint8_t *data)
{
  if (CIRC_CNT(circ->head, circ->tail, circ->size) == 0)
    return -1;

  *data = circ->buf[circ->tail];
  circ->tail = (circ->tail + 1) & (circ->size - 1);
  return 0;
}

int circ_get3(struct circ_buf* circ, uint8_t *data)
{
  if (CIRC_CNT(circ->head, circ->tail, circ->size) == 0)
    return -1;

  *data = circ->buf[circ->tail];
  circ->tail = (circ->tail + 1) & (circ->size - 1);
  return 0;
}
