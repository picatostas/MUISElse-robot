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

enum collision_state{
	COLLISION_IDLE,
	COLLISION_BACKWARDS,
	COLLISION_TURN
};


int collision_detected = 0,wall_is_near = 0,wall_is_lost = 0;




static int wall_collision(fsm_t * this){

	return collision_detected;
}
static int wall_near(fsm_t * this){

	return wall_is_near;
}
static int wall_lost(fsm_t * this){

	return wall_is_lost;
}
static void go_backwards(fsm_t * this){

 // codigo para marcha atras
}
static void turn_right(fsm_t * this){

// codigo para girar drch

}


static fsm_trans_t collision_tt[] = {
  { 	 COLLISION_IDLE,		 wall_collision, COLLISION_BACKWARDS, 		  NULL},
  { COLLISION_BACKWARDS,		 wall_collision, COLLISION_BACKWARDS, go_backwards},
  { COLLISION_BACKWARDS,		 	  wall_near, 	  COLLISION_TURN, 		  NULL},
  { 	 COLLISION_TURN,		 	  wall_near, 	  COLLISION_TURN,   turn_right},
  { 	 COLLISION_TURN,		 	  wall_lost, 	  COLLISION_IDLE,  		  NULL},
  {-1, NULL, -1, NULL }
};


fsm_t* fsm_new_collision() {
	return fsm_new(collision_tt);
}

