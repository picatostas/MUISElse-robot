#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "ring_buf.h"
#include "fsm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f411e_discovery_gyroscope.h"
#include "stm32f411e_discovery_accelerometer.h"
#include "stm32f411e_discovery_magnetometer.h"
#include "gyro.h"
#include "accelero.h"
#include "magneto.h"
#include "parser.h"
#include "sensor.h"
#include "uart.h"
#include "usb.h"
#include <stddef.h>
#include "main.h"
//local values for parser fsm
static Received_msgs  msgs;
static flags cmd = (flags){0,0,0,0,0};

//global values mentions
//				TILT,PAN,LEFT,RIGHT
int pwm_values[]={TILT_DEF,PAN_DEF,LEFT_DEF,RIGHT_DEF};
extern TIM_HandleTypeDef htim4,htim3,htim10;
extern rb_struct rb_usb;
extern rb_struct rb_uart;

//map function
static uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);

uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  return (uint16_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

//fsm states definitions
enum parser_state{
	PARSER_IDLE,
	PARSER_READFIRST,
	PARSER_A,
	PARSER_M,
	PARSER_L,
	PARSER_S
};

//inputs functions definitions
static int return_true(fsm_t* this){return 1;}

static int check_command_count(fsm_t * this){

	if (msgs.count > 0) {
		return 1;
	}
	else
	return 0;
}

static int cmd_IMU(fsm_t* this){

	return cmd.IMU;
}

static int cmd_M(fsm_t* this){

	return cmd.M;
}

static int cmd_L(fsm_t* this){

	return cmd.L;
}

static int cmd_S(fsm_t* this){

	return cmd.S;
}

//outputs functions definitions
static void do_IMU(fsm_t* this){

	int send_rate = 0;
	unsigned char str[4];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=2;
	strncpy(str,pv,3);
	send_rate = atoi(str);
	send_rate = map(send_rate,0,999,200,999);
	sensorIMU.period = (uint32_t)send_rate;
	cmd.IMU = 0;
}

static void do_M(fsm_t* this){

	unsigned char str[3];
	unsigned char l[2],r[2];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=3;
	strncpy(str,pv,2);
	strncpy(l,pv,1);
	pv+=1;
	strncpy(r,pv,1);
	int pwm_value_temp = atoi(str);
	int left=atoi(l);
	int right=atoi(r);
	pwm_value_temp = map(pwm_value_temp,0,90,50,250);
	//left = map(left,0,9,50,250);
	//right = map(right,0,9,50,250);
	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count][2];
	switch (select_char) {
		case '0':
			pwm_values[0]=pwm_value_temp;
			break;
		case '1':
			pwm_values[1]=pwm_value_temp;
			break;
		case '2':
			pwm_values[2]=pwm_value_temp;
			break;
		case '3':
			pwm_values[3]=pwm_value_temp;
			break;
		case '4':
			switch (left) {
				case 0:
					pwm_values[2]=50;
					break;
//				case 2:
//					pwm_values[2]=100;
//					break;
				case 5:
					pwm_values[2]=150;
					break;
//				case 7:
//					pwm_values[2]=200;
//					break;
				case 9:
					pwm_values[2]=250;
					break;
				default:
					break;
			}
			switch (right) {
				case 0:
					pwm_values[3]=50;
					break;
//				case 2:
//					pwm_values[3]=100;
//					break;
				case 5:
					pwm_values[3]=150;
					break;
//				case 7:
//					pwm_values[3]=200;
//					break;
				case 9:
					pwm_values[3]=250;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	HAL_TIM_Base_Start_IT(&htim10);

	cmd.M = 0;
}

static void do_L(fsm_t* this){

	int pwm_value = 0;
	unsigned char str[3];
	unsigned char * pv = msgs.buffer[msgs.count];
	pv+=3;
	strncpy(str,pv,2);
	//int pwm_value_temp = atoi(str);
	pwm_value = atoi(str);
	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count][2];
	switch (select_char) {
		case '0':
			//pwm_values[0]=pwm_value_temp;
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,pwm_value);
			break;
		case '1':
			//pwm_values[1]=pwm_value_temp;
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2,pwm_value);
			break;
		case '2':
			//pwm_values[2]=pwm_value_temp;
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_3,pwm_value);
			break;
		case '3':
			//pwm_values[3]=pwm_value_temp;
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,pwm_value);
			break;
		default:
			break;
	}
	cmd.L = 0;
}

static void do_S(fsm_t* this){

	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count][2];
	unsigned char send_usb    =(unsigned char) msgs.buffer[msgs.count][3];
	unsigned char send_uart   =(unsigned char) msgs.buffer[msgs.count][4];
	switch (select_char){
		case 'A':
			if (send_usb  == '1')sensorIMU.usb = 1;else  sensorIMU.usb = 0;
			if (send_uart == '1')sensorIMU.uart= 1;else sensorIMU.uart = 0;
			break;
		default:
			break;
	}



	cmd.S = 0;
}

static void select_device(fsm_t * this){

	cmd.IMU = 0;
	cmd.M = 0;
	cmd.L = 0;
	cmd.S = 0;
	unsigned char select_char =(unsigned char) msgs.buffer[msgs.count -1][1];

	switch (select_char){
		case 'A':
			cmd.IMU = 1;
			break;
		case 'M':
			cmd.M = 1;
			break;
		case 'L':
			cmd.L = 1;
			break;
		case 'S':
			cmd.S = 1;
			break;
		default:
			break;
	}
	msgs.count--;

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if (htim->Instance == TIM10){
		HAL_TIM_Base_Stop_IT(&htim10);
		uint32_t pwm_increasing = 5;
		uint32_t future_value[4];

			for (int i = 0; i < 4; i++) {
				uint32_t AUX_CH;
				switch (i) {
					case 0:
						AUX_CH = TIM_CHANNEL_1;
						break;
					case 1:
						AUX_CH = TIM_CHANNEL_2;
						break;
					case 2:
						AUX_CH = TIM_CHANNEL_3;
						break;
					case 3:
						AUX_CH = TIM_CHANNEL_4;
						break;
					default:
						break;
				}
				if (pwm_values[i] == __HAL_TIM_GET_COMPARE(&htim3,AUX_CH)) {
					continue;
				}
				if (pwm_values[i] > __HAL_TIM_GET_COMPARE(&htim3,AUX_CH)){ //new value differs from original
					future_value[i] = __HAL_TIM_GET_COMPARE(&htim3,AUX_CH) + pwm_increasing;
					if (future_value[i] < pwm_values[i]){
						__HAL_TIM_SET_COMPARE(&htim3,AUX_CH,future_value[i]);
					}
					else{
						__HAL_TIM_SET_COMPARE(&htim3,AUX_CH,pwm_values[i]);
					}
				}
				else {
					future_value[i] = __HAL_TIM_GET_COMPARE(&htim3,AUX_CH) - pwm_increasing;
					if (future_value[i] > pwm_values[i]){
						__HAL_TIM_SET_COMPARE(&htim3,AUX_CH,future_value[i]);
					}
					else{
						__HAL_TIM_SET_COMPARE(&htim3,AUX_CH,pwm_values[i]);
					}
				}

			}
			HAL_TIM_Base_Start_IT(&htim10);
		}


}



//transiitons rules mentions
static fsm_trans_t parser_tt[] = {

  { PARSER_IDLE, check_command_count, PARSER_READFIRST, select_device},
  { PARSER_READFIRST,cmd_IMU, PARSER_A,do_IMU },
  { PARSER_READFIRST,cmd_M, PARSER_M,  do_M },
  { PARSER_READFIRST,cmd_L, PARSER_L,  do_L },
  { PARSER_READFIRST,cmd_S, PARSER_S,  do_S },
  { PARSER_A,return_true, PARSER_IDLE,  NULL },
  { PARSER_M,return_true, PARSER_IDLE,  NULL },
  { PARSER_L,return_true, PARSER_IDLE,  NULL },
  { PARSER_S,return_true, PARSER_IDLE,  NULL },
  {-1, NULL, -1, NULL }
};
fsm_t* fsm_new_parser() {
	return fsm_new(parser_tt);
}

//Message check function
void parseMsg(rb_struct * rb){

	uint16_t size;
	static unsigned char usb_buf[] = "tx0: RA000\r\n",uart_buf[] = "tx0: RA000\r\n";
		size = rb_len(rb);

		if (size > 0) {
			while (size && rb_read(rb, 0) != 'R') {
				rb_pop(rb);
				size--;
			}
		}
		if (size > 5) {

			uint8_t i;
			for (i=0; i<5; i++) {
				msgs.buffer[msgs.count][i] = rb_read(rb,0);
				if(rb == &rb_uart) uart_buf[i+5] = rb_pop(rb);
				if(rb == &rb_usb) usb_buf[i+5] = rb_pop(rb);
			}

			msgs.count++;
			if (msgs.count >= MAX_MSG) {
				msgs.count = MAX_MSG - 1;
			}
			size -= 5;
			while (size && (rb_pop(rb) != '\r')) {
				size--;
			}


		}


}

