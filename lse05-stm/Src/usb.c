#include "fsm.h"
#include "usb.h"
#include <stddef.h>
#include "parser.h"
//state definitions
enum usb_state{
	USB_IDLE,
	USB_PARSE
};

//value definitions for usb fsm
uint32_t usb_count=0;
uint8_t usb_flag=0 ;
rb_struct rb_usb;


//input
static int usb_timer(fsm_t* this){
	return usb_flag;
}
static int usb_parse_finished(fsm_t* this){
	return !usb_flag;
}

//output
static void usb_parse(fsm_t* this){
	parseMsg(&rb_usb);
	usb_flag = 0;
	usb_count = 0;
}

//transitions rules
static fsm_trans_t usb_tt[] = {
  { USB_IDLE, usb_timer, USB_PARSE,     usb_parse},
  { USB_PARSE,    usb_parse_finished, USB_IDLE,  NULL },
  {-1, NULL, -1, NULL }
};

fsm_t* fsm_new_usb() {
	return fsm_new(usb_tt);
}

// usb interruption
void LSE_Receive_callback(uint8_t* Buf, uint32_t Len){
	while(Len--){
		rb_write(&rb_usb, *Buf++);
	}

}
