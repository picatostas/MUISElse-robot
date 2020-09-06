# MUISElse-robot
Code I wrote with my colleagues for Embedded Systems Lab during our time in the Masters Degree in Embedded Electronics Systems. 
I am author of those commits with "pablo".
Colleagues:
Jose Antonio Moral Parras
Daniel Gala Montes
Silvia Hervas
Filippo Mangani

The project consisted in a ROS enviroment to control a pseudo autonomous robot.

## STM32DiscoveryF4

The STM32 was used for fetching data from the sensors: Gyro, Magnetometer and Accelerometer. This sensors were interfaced by I2C using BSP HAL library.
The sensor data was later send by serial via USB to the Raspberry Pi.
STM32 received commands to move the motors. Several sets of FSM were interacting with each other to manage all the resources: sensors, adc, motors, etc.

### Motors
Commands recieved from the raspberry Pi were turned into PWM to control the 4 motors.

#### Modified servos as Continuous rotation
These motors were the Left and Right wheels of the robot, as a third point of support, a crazy wheel was installed on the front of the robot.

#### Microservos
2 Microservos were used for controlling the camera turret.

## Raspberry Pi

A Raspberry Pi was used for running ROS. Running the Ros_node teleopjoy subscribing to the commands sent by the computer.
raspicam_node was used to stream live video from the Raspicam as well as running OpenCV for detecting the position of 2 targets, green and red color, respectively.
A pseudo-autonomous behavior was scheduled for orientating the robot with STM32 streamed data to later reach the targets calculating their position via OpenCV.

## Linux Laptop

The laptop was used to run ros_nodes for the video visualization streamed by the raspicam_node and to connect a remote controller to send commands to the Raspberry Pi through the teleopjoy ros_topic. 


![robot-img](/robot-lsel.png?raw=true "Robot Lsel")
