#!/usr/bin/env python

import rospy
import serial
import tf, tf_conversions

from sensor_msgs.msg   import Imu
from geometry_msgs.msg import Vector3
from geometry_msgs.msg import Quaternion

serial_port 			= 	'/dev/ttyUSB0'
baudrate 				=	115200
serial_timeout_seconds  = 	1
sensors_list 			= 	['A', 'C', 'G']
sensors_dictionary		= 	{'A' : 'acceleration', 'G' : 'gyroscope', 'C' : 'compass'}
command_min_length		=	5

#Covariance matrixes
orientation_covariance         = [-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
angular_velocity_covariance	   = [-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
linear_acceleration_covariance = [-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

#Sensors data ROS buffers
orientation_msg = Vector3(0, 0, 0)
angular_vel_msg = Vector3(0, 0, 0)
lin_accel_msg   = Vector3(0, 0, 0)


#Sensors data dictionary
sensor_data_dict = {'A' : lin_accel_msg, 'G' : angular_vel_msg, 'C' : orientation_msg}

#ROS IMU message
imuMsg = Imu()
imuMsg.orientation_covariance		  = orientation_covariance
imuMsg.angular_velocity_covariance 	  = angular_velocity_covariance
imuMsg.linear_acceleration_covariance = linear_acceleration_covariance
imuMsg.orientation 					  = orientation_msg
imuMsg.angular_velocity 			  = angular_vel_msg
imuMsg.linear_acceleration 			  = lin_accel_msg

"""
Node must be able of receiving STM32 sensor messages through
serial port, and transform them to a ROS IMU message. The data
received through the serial port has the following text format:

<Type> ':' <Data> <Data> ... <Data> '\r' '\n'

"Type" must be one of these letters: A (accelerometer), C (compass),
G (gyroscope).
"""

def IMU_publisher():
	#ROS message publisher
	pub = rospy.Publisher('imu_pub', Imu, queue_size = 1)
	rospy.init_node('IMU_publisher')
	rate = rospy.Rate(10) #10Hz
	print_msg = True

	while not rospy.is_shutdown():	
	#try:
		with serial.Serial(serial_port, baudrate, timeout=serial_timeout_seconds) as ser:

			line = ser.readline()  #new command received
			if (len(line) > command_min_length):
				sensor_type = line[0]  #read first character
				for element in sensors_list:
					if element == sensor_type:
						print 'Updating ' + sensors_dictionary[element] + ' sensor value...'
						new_sensor_values = line[2:]
						new_sensor_values_splitted = new_sensor_values.split(',')
						#Update values
						sensor_data_dict[element].x = float(new_sensor_values_splitted[0])
						sensor_data_dict[element].y = float(new_sensor_values_splitted[1])
						sensor_data_dict[element].z = float(new_sensor_values_splitted[2])
				#Write values to IMU ROS message
				imuMsg.orientation 		    = Quaternion(*tf_conversions.transformations.quaternion_from_euler(orientation_msg.x, orientation_msg.y, orientation_msg.z))
				imuMsg.angular_velocity	    = angular_vel_msg
				imuMsg.linear_acceleration  = lin_accel_msg

				#Publish ROS topic
				pub.publish(imuMsg)
				rate.sleep()
	
	"""
	except serial.SerialException:

		if print_msg == True:
			print 'Error opening serial port! Please check if ' \
				'it is the right one.'
			print_msg = False
	"""

if __name__ == '__main__':
	try:

		IMU_publisher()
	except rospy.ROSInterruptException:
		pass
