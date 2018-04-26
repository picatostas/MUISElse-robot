#include <stdint.h>
#include "ring_buf.h"
  
/**
  * @brief  Initializes the ring buffer.
  * @param  this: ring buffer instance
  * @retval None
  */
void
rb_setup(rb_struct* this) {
  this->rd_index = 0;
  this->wr_index = 0;
}

/**
  * @brief  Insert data in ring buffer.
  * @param  this: ring buffer instance
  * @param  data: char inserted to the buffer
  * @retval None
  */
void
rb_write(rb_struct* this, uint8_t data) {
  *(this->buf+this->wr_index++) = data;
  if (this->wr_index >= RB_SIZE) {
    this->wr_index = 0;
  }
}

/**
  * @brief  Check number of data stored in ring buffer.
  * @param  this: ring buffer instance
  * @retval number of bytes stored
  */
uint16_t
rb_len(rb_struct* this) {
  uint16_t result = this->wr_index - this->rd_index;
  if (result > this->wr_index) {
    result += RB_SIZE;
  }
  return result;
}

/**
  * @brief  Read a byte from buffer, keeping it in the buffer.
  *         Check availability before calling this function
  * @param  this: ring buffer instance
  * @param  offset: position of the value to be read
  * @retval value at offset position
  */
uint8_t
rb_read(rb_struct* this, uint16_t offset) {
  uint16_t target = (this->rd_index+offset)%RB_SIZE;
  return *(this->buf+target);
}

/**
  * @brief  Read and remove the first byte from the buffer.
  *         Check availability before calling this function
  * @param  this: ring buffer instance
  * @retval first value of the buffer
  */
uint8_t
rb_pop(rb_struct* this) {
  uint8_t result;
  result = this->buf[this->rd_index++];
  if (this->rd_index >= RB_SIZE) {
    this->rd_index -= RB_SIZE;
  }
  return result;
}