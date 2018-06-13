#ifndef SENSOR_H
#define SENSOR_H

#define RECEIVE_TIMEOUT 20
//structs definitions
typedef struct{
	uint8_t IMU;
	uint8_t P;
	uint8_t M;
	uint8_t L;
	uint8_t S;
}flags;

typedef struct
{
    uint32_t       count;
	uint32_t      period;
	uint8_t 	 	 usb;
	uint8_t 		uart;
} Sensor;
Sensor sensorIMU,sensorP;

typedef struct {
		int16_t x;
		int16_t y;
		int16_t z;
}DATA_ACCELERO;

typedef struct {
		float x;
		float y;
		float z;
}DATA_GYRO;

typedef struct {
	    float x;
	    float y;
	    float z;

}DATA_COMP;

// mention global value huart2 from main
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef hadc1;
//fsm definition
fsm_t* fsm_new_imu();
fsm_t* fsm_new_proximity();
#endif
