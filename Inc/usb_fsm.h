#ifndef USB_FSM_H_
#define USB_FSM_H_
#include "fsm.h"
#include "main.h"
#include <signal.h>
#include "ring_buf.h"

enum usb_state{
	USB_IDLE,
	USB_PARSE
};
extern uint8_t usb_flag,usb_count;
extern void parseMsg(rb_struct *);
extern rb_struct rb_usb;


fsm_t * usb_fsm_new(void);
int usb_timer(fsm_t* this);
void usb_parse(fsm_t* this);
int usb_parse_finished(fsm_t* this);
#endif

fsm_trans_t parser_tt[] = {

  { 	PARSER_IDLE, NULL, PARSER_READFIRST, NULL},
  { PARSER_READFIRST,cmd_A, PARSER_A,  NULL },
  { PARSER_READFIRST,cmd_C, PARSER_C,  NULL },
  { PARSER_READFIRST,cmd_G, PARSER_G,  NULL },
  { PARSER_READFIRST,cmd_T, PARSER_T,  NULL },
  { PARSER_READFIRST,cmd_M, PARSER_M,  NULL },
  { PARSER_READFIRST,cmd_L, PARSER_L,  NULL },
  { PARSER_READFIRST,cmd_S, PARSER_S,  NULL },
  { 		PARSER_A,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_C,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_G,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_T,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_M,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_L,NULL, PARSER_IDLE,  NULL },
  { 		PARSER_S,NULL, PARSER_IDLE,  NULL },
  {-1, NULL, -1, NULL }
};