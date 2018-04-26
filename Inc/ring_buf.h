#ifndef _RING_BUF_H

#define _RING_BUF_H

#define RB_SIZE 256

typedef struct _rb_struct {
  uint8_t buf[RB_SIZE];
  uint16_t rd_index;
  uint16_t wr_index;
} rb_struct;

void rb_setup(rb_struct* this);
void rb_write(rb_struct* this, uint8_t data);
uint16_t rb_len(rb_struct* this);
uint8_t rb_read(rb_struct* this, uint16_t offset);
uint8_t rb_pop(rb_struct* this);

#endif //_RING_BUF_H
