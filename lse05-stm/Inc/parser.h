#ifndef parser_H
#define parser_H

//for use of pointer *rb

#include "ring_buf.h"

//message size definitions
#define MSG_SIZE 6
#define UART_MSG_SIZE 6
#define MAX_MSG 3

typedef struct{
	unsigned char buffer[MAX_MSG][MSG_SIZE];
	uint8_t count;
}Received_msgs;

//Parser function first defined here
void parseMsg(rb_struct * rb);

//fsm_parswe define
fsm_t* fsm_new_parser();
#endif
