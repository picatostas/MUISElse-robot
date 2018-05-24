#!/usr/bin/env python
import rospy
import serial
from sensor_msgs.msg import Joy

# Author: Daniel Gala
# This ROS Node converts Joystick inputs from the joy node
# into serial commands to drive the robot LSEL05

# Store the last commands to avoid sending useless messages
last_left = [0, 0] # [angular,linear]
last_right = [0, 0] # [angular,linear]

# Store the current camera position
tilt = 0 # values: [-1,0,1] positions: [up,mid,down]
pan = 0 # values: [-1,0,1] positions: [left,mid,right]

# Receives joystick messages (subscribed to Joy topic)
# then converts the joysick inputs into serial commands
def callback(data):

    global last_left
    global last_right
    global tilt
    global pan

    ser = serial.Serial('/dev/ttyUSB0',115200)

    # horizontal left stick axis = turn rate
    left_angular = data.axes[0]
    # vertical left stick axis = linear rate
    left_linear = data.axes[1]
    # horizontal right stick axis = turn rate
    right_angular = data.axes[2]
    # vertical right stick axis = linear rate
    right_linear = data.axes[3]

    # when the left joystick is moved
    # if its not a repeated message
    if (left_angular != last_left[0] or left_linear != last_left[1]):
	# update last state
        last_left[0] = left_angular
        last_left[1] = left_linear

        # in resting state, both wheels stay still
        if left_angular == 0 and left_linear == 0:
	    ser.write('RM455\r\n')
	    ser.write('RM455\r\n')
        # when the left joystick is moved vertically only
        # both wheels move at full speed
        elif left_angular == 0:
            # forwards
            if left_linear > 0:
		ser.write('RM409\r\n')
            # or backwards
            elif left_linear < 0:
		ser.write('RM490\r\n')
        # when the left joystick is moved horizontally only
        # both wheels move at full speed in opposite senses
        elif left_linear == 0:
            if left_angular > 0:
		ser.write('RM499\r\n')
            elif left_angular < 0:
		ser.write('RM400\r\n')
        # when the left joystick is moved any other way
        # one wheel runs faster than the other depending on the desired movement
        else:
            if left_angular * left_linear > 0:
		ser.write('RM469\r\n')
            elif left_angular > 0 and left_linear < 0:
		ser.write('RM430\r\n')
            elif left_angular < 0 and left_linear > 0:
		ser.write('RM496\r\n')
            else:
		ser.write('RM403\r\n')

    # when the right joystick is moved
    elif (right_angular != last_right[0]) or (right_linear != last_right[1]):
        # ignore intermediate analog values to avoid subsequent contradictory messages
    	if ((abs(right_angular)%1) == 0) and ((abs(right_linear)%1) == 0):
            last_right[0] = right_angular
            last_right[1] = right_linear
            # in resting state, the camera stays still
            if right_angular == 0 and right_linear == 0:
                pass
            # when the right joystick is moved vertically only
            # the camera tilts
            elif right_angular == 0:
                # forwards
                if right_linear > 0:
                    if tilt == 0:
                        ser.write('RM060\r\n')
                        ser.write('RM060\r\n')
                        tilt = -1
                    elif tilt > 0:
                        ser.write('RM075\r\n')
                        ser.write('RM075\r\n')
                        tilt = 0
                        
                # or backwards
                elif right_linear < 0:
                    if tilt == 0:
                        ser.write('RM090\r\n')
                        ser.write('RM090\r\n')
                        tilt = 1
                    elif tilt < 0:
                        ser.write('RM075\r\n')
                        ser.write('RM075\r\n')
                        tilt = 0

            # when the right joystick is moved horizontally only
            # the camera rotates
            elif right_linear == 0:
                # counter-clockwise
                if right_angular > 0:
                    if pan == 0:
                        ser.write('RM175\r\n')
                        ser.write('RM175\r\n')
                        pan = -1
                    elif pan > 0:
                        ser.write('RM140\r\n')
                        ser.write('RM140\r\n')
                        pan = 0

                # or clockwise
                elif right_angular < 0:
                    if pan == 0:
                        ser.write('RM115\r\n')
                        ser.write('RM115\r\n')
                        pan = 1
                    elif pan < 0:
                        ser.write('RM140\r\n')
                        ser.write('RM140\r\n')
                        pan = 0

            # when the right joystick is moved any other way
            # the camera doesn't move
    else:
    # a button has been pressed, not implemented
       pass

    ser.close()

# Intializes everything
def start():
    # subscribed to joystick inputs on topic "joy"
    rospy.Subscriber("joy", Joy, callback)
    # starts the node
    rospy.init_node('teleop_joy')
    rospy.spin()

if __name__ == '__main__':
    start()

