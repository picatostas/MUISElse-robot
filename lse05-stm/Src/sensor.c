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
#include "main.h"
//timers for sensor definitions
flags sensor_timer = (flags){0,0,0,0,0,0,0,0};

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
static int timer_finished_A(fsm_t* this){
	return !sensor_timer.A;

}
static void send_A(fsm_t* this){
	static unsigned char imu_buf[85];
		int16_t accelero[3]= {0};
		DATA_ACCELERO data_ACC;
		 	BSP_ACCELERO_GetXYZ(accelero);
			data_ACC.x = accelero[0];
			data_ACC.y = accelero[1];
			data_ACC.z = accelero[2];
			float compass[3]={0};
			DATA_COMP data_COMP;
				    BSP_MAGNETO_GetXYZ(compass);
					data_COMP.x = compass[0];
					data_COMP.y = compass[1];
					data_COMP.z = compass[2];
					float gyroscope[3]= {0};
						DATA_GYRO data;
						BSP_GYRO_GetXYZ(gyroscope);
						data.x = gyroscope[0];
						data.y = gyroscope[1];
						data.z = gyroscope[2];
			sprintf(imu_buf,"S:%06d,%06d,%06d,%08.2f,%08.2f,%08.2f,%08.2f,%08.2f,%08.2f\r\n",data_ACC.x,data_ACC.y,data_ACC.z,data_COMP.x,data_COMP.y,data_COMP.z,data.x,data.y,data.z);

				if (sensorA.uart){
					HAL_UART_Transmit(&huart2, (uint8_t *)imu_buf,sizeof(imu_buf)-1,10);

				}
				if (sensorA.usb){
					if (USBD_CDC_SetTxBuffer(&hUsbDeviceFS, imu_buf, sizeof(imu_buf)-1)==USBD_OK){
						if (USBD_CDC_TransmitPacket(&hUsbDeviceFS)==USBD_OK){
						}
					}

				}
			sensorA.count = 0;
				sensor_timer.A = 0;
}


static fsm_trans_t imu_tt[] = {

  { SENSOR_IDLE, timer_A, SENSOR_SEND,send_A},
  { SENSOR_SEND,timer_finished_A, SENSOR_IDLE,  NULL },
  {-1, NULL, -1, NULL }

};

fsm_t* fsm_new_imu() {
	return fsm_new(imu_tt);
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

}
