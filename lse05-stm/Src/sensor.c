#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "ring_buf.h"
#include "fsm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f411e_discovery_gyroscope.h"
#include "stm32f411e_discovery_accelerometer.h"
#include "gyro.h"
#include "accelero.h"
#include "magneto.h"
#include "parser.h"
#include "sensor.h"
#include "uart.h"
#include "usb.h"
#include <stddef.h>
//timers for sensor definitions
flags sensor_timer = (flags){0,0,0,0,0,0,0};

//global values
extern uint32_t uart_count;
extern uint8_t uart_flag;
extern uint8_t usb_flag;
extern uint32_t usb_count;

//sensor fsm states definitions
enum sensor_state{
	SENSOR_IDLE,
	SENSOR_SEND
};

//inputs definitions
static int timer_A(fsm_t* this){
	return sensor_timer.A;
}
static int timer_G(fsm_t* this){
	return sensor_timer.G;
}
static int timer_finished_A(fsm_t* this){
	return !sensor_timer.A;

}
static int timer_finished_G(fsm_t* this){
	return !sensor_timer.G;

}

//outputs definitions
static void send_A(fsm_t* this){

	int16_t accelero[3]= {0};

	DATA_ACCELERO data;
 	BSP_ACCELERO_GetXYZ(accelero);
	data.x = accelero[0];
	data.y = accelero[1];
	data.z = accelero[2];
	static unsigned char accelero_buf[] = "A:XXXXXX,YYYYYY,ZZZZZZ\r\n";
	sprintf(accelero_buf,"A:%06d,%06d,%06d\r\n",data.x,data.y,data.z);

	if (sensorA.uart){
		HAL_UART_Transmit(&huart2, (uint8_t *)accelero_buf,sizeof(accelero_buf)-1,10);

	}
	if (sensorA.usb){
		if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, accelero_buf, sizeof(accelero_buf)-1)==USBD_OK){
			if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
			}
		}

	}
	sensorA.count = 0;
	sensor_timer.A = 0;
}
static void send_G(fsm_t* this){

	float gyroscope[3]= {0};

	DATA_GYRO data;

	BSP_GYRO_GetXYZ(gyroscope);

	data.x = gyroscope[0];
	data.y = gyroscope[1];
	data.z = gyroscope[2];
	static unsigned char gyroscope_buf[] = "G:XXXXXXXX,YYYYYYYY,ZZZZZZZZ\r\n";
	sprintf(gyroscope_buf,"G:%08.2f,%08.2f,%08.2f\r\n",data.x,data.y,data.z);

	if (sensorG.uart) {
		HAL_UART_Transmit(&huart2, (uint8_t *)gyroscope_buf,sizeof(gyroscope_buf)-1,10);

	}
	if(sensorG.usb){
		if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, gyroscope_buf, sizeof(gyroscope_buf)-1)==USBD_OK){
			if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
			}
		}
	}
	sensorG.count = 0;
	sensor_timer.G = 0;
}

// transition rules
static fsm_trans_t accelero_tt[] = {

  { SENSOR_IDLE, timer_A, SENSOR_SEND,send_A},
  { SENSOR_SEND,timer_finished_A, SENSOR_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};

fsm_t* fsm_new_accelero() {
	return fsm_new(accelero_tt);
}

static fsm_trans_t gyro_tt[] = {

  { SENSOR_IDLE, timer_G, SENSOR_SEND,send_G},
  { SENSOR_SEND,timer_finished_G, SENSOR_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};
fsm_t* fsm_new_gyro() {
	return fsm_new(gyro_tt);
}

// fsm_usb interruption
void HAL_SYSTICK_Callback(void){

	if (usb_flag == 0) {
		usb_count++;
		if (usb_count >= 500) {
			usb_flag = 1;
		}
	}
	if (uart_flag == 0) {
		uart_count++;
		if (uart_count >= 500) {
			uart_flag = 1;
		}
	}
	if (sensor_timer.A == 0) {
			sensorA.count++;
			if (sensorA.count >= sensorA.period) {
				sensor_timer.A = 1;
			}

		}

	if (sensor_timer.G == 0) {
			sensorG.count++;
			if (sensorG.count >= sensorG.period) {
				sensor_timer.G = 1;
			}
		}
}
