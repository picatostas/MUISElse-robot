/*
 * collision.c
 *
 *  Created on: 12 jun. 2018
 *      Author: pablo
 */
#include "collision.h"
#include "fsm.h"
#include <stddef.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "parser.h"
//#include "stm32f4xx_hal_uart.h"

enum collision_state{
	COLLISION_IDLE,
	COLLISION_BACKWARDS,
	COLLISION_TURN
};


int collision_detected = 0,wall_is_near = 0,wall_is_lost = 0;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;
extern float heading;
extern uint8_t uart_rx[512];
float current_heading = 0, target_heading = 0;

static int wall_collision(fsm_t * this){

	return collision_detected;
}
static int wall_near(fsm_t * this){

	if (wall_is_near) {
		current_heading = heading;
		target_heading = current_heading + 90;
		if(target_heading > 360) target_heading -= 360;
	}
	return wall_is_near;
}
static int wall_not_near(fsm_t * this){

	return !wall_is_near;
}
static int wall_lost(fsm_t * this){


	return wall_is_lost;
}
static int wall_not_lost(fsm_t * this){


	return !wall_is_lost;
}
static void go_backwards(fsm_t * this){

	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,200);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,100);
 // codigo para marcha atras
}
static void turn_right(fsm_t * this){




	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,100);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,100);

	if (abs(target_heading-heading) < 10) {
		wall_is_lost=1;
	}
// codigo para girar drch

}
static void stop_UART(fsm_t * this){

	HAL_UART_Abort_IT(&huart2);
}
static void start_UART(fsm_t * this){

	HAL_UART_Receive_IT(&huart2,(uint8_t *)uart_rx,UART_MSG_SIZE);
}
static fsm_trans_t collision_tt[] = {
  { 	 COLLISION_IDLE,		 wall_collision, COLLISION_BACKWARDS,    stop_UART},
  { COLLISION_BACKWARDS,		  wall_not_near, COLLISION_BACKWARDS, go_backwards},
  { COLLISION_BACKWARDS,		 	  wall_near, 	  COLLISION_TURN, 		  NULL},
  { 	 COLLISION_TURN,		  wall_not_lost, 	  COLLISION_TURN,   turn_right},
  { 	 COLLISION_TURN,		 	  wall_lost, 	  COLLISION_IDLE,   start_UART},
  {-1, NULL, -1, NULL }
};


fsm_t* fsm_new_collision() {
	return fsm_new(collision_tt);
}

