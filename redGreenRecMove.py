# USAGE
# python ball_tracking.py --video ball_tracking_example.mp4
# python ball_tracking.py

# import the necessary packages
from math import atan
from math import sqrt

from collections import deque
import numpy as np
import argparse
import imutils
import cv2


# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video",
	help="path to the (optional) video file")
ap.add_argument("-b", "--buffer", type=int, default=64,
	help="max buffer size")
args = vars(ap.parse_args())

# define the lower and upper boundaries of the "green"
# ball in the HSV color space, then initialize the
# list of tracked points
greenLower = (29, 86, 6)
greenUpper = (64, 255, 255)
#redLower = (200, 200, 100)
#redUpper = (255,0 , 50)
redLower = ( 150, 100, 150)
redUpper = ( 255, 255, 255)
goRed = 0
goGreen = 0
pts = deque(maxlen=args["buffer"])
rosCamg = open ('./rosCamg', 'w+')
rosCamg.truncate()
rosWheelg = open ('./rosWheelg', 'w+')
rosWheelg.truncate()
rosCamr = open ('./rosCamr', 'w+')
rosCamr.truncate()
rosWheelr = open ('./rosWheelr', 'w+')
rosWheelr.truncate()

distancia = open('./distancia','w+')
distancia.truncate()


import serial
ser = serial.Serial()
ser = serial.Serial('/dev/ttyUSB0', 115200)

# if a video path was not supplied, grab the reference
# to the webcam
if not args.get("video", False):
	camera = cv2.VideoCapture(0)

# otherwise, grab a reference to the video file
else:
	camera = cv2.VideoCapture(args["video"])

# keep looping


while True:

	# grab the current frame
	(grabbed, frame) = camera.read()

	# if we are viewing a video and we did not grab a frame,
	# then we have reached the end of the video
	if args.get("video") and not grabbed:
		break

	# resize the frame, blur it, and convert it to the HSV
	# color space
	frame = imutils.resize(frame, width=600)
        # draw thirds lines
        height, width, channels = frame.shape
        cv2.line(frame,(width/3,0),(width/3,height),(255,0,0),1)
        cv2.line(frame,(width*2/3,0),(width*2/3,height),(255,0,0),1)
        cv2.line(frame,(0,height/3), (width,height/3),(255,0,0),1)
        cv2.line(frame,(0,height*2/3),(width,height*2/3),(255,0,0),1)
	# blurred = cv2.GaussianBlur(frame, (11, 11), 0)
	hsv  = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        hsv2 = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
	# construct a mask for the color "green", then perform
	# a series of dilations and erosions to remove any small
	# blobs left in the mask
	mask = cv2.inRange(hsv, greenLower, greenUpper)   
	mask = cv2.erode(mask, None, iterations=2)
	mask = cv2.dilate(mask, None, iterations=2)
	mask2 = cv2.inRange(hsv,redLower,redUpper)
	mask2 = cv2.erode(mask2,None,iterations=2)
	mask2 = cv2.dilate(mask2,None,iterations=2)
         
	# find contours in the mask and initialize the current
	# (x, y) center of the ball
	cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,	cv2.CHAIN_APPROX_SIMPLE)[-2]
	center = None
	cnts2= cv2.findContours(mask2.copy(),cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)[-2]
	center = None
	if len(cnts2) > 0  and len(cnts) > 0:
		c3 = max(cnts2, key=cv2.contourArea)
		((x4, y4), radiuAs) = cv2.minEnclosingCircle(c3)
		c4 = max(cnts, key=cv2.contourArea)
		((x5, y5), radiuSs) = cv2.minEnclosingCircle(c4)
		xfin = x4 - x5
		yfin = y4 - y5
	        distanciafin = sqrt(yfin*yfin + xfin*xfin)
		#print >>angulof ,angulo 
		if xfin > 0 : 
			print >>distancia , distanciafin
		else:
			print >>distancia , -distanciafin

	# only proceed if at least one contour was found
	if len(cnts2) > 0:
		c2 = max(cnts2, key=cv2.contourArea)
		((x1, y2), radius) = cv2.minEnclosingCircle(c2)
		M2 = cv2.moments(c2)
		center2=(int (M2["m10"]/M2["m00"]),int(M2["m01"]/ M2["m00"])) 
		joy_new.axes[0]=0.0	
		# draw the circle and centroid on the frame,
		# then update the list of tracked points
		if radius > 10:
			if int(center2[0])<(width*2/3) and int(center2[0]) > (width/3) and int(center2[1])<(height*2/3) and int(center2[1])>(height/3):
#				print >>rosWheelr, 'GO'
				ser.write('RM409\r\n')
			cv2.circle(frame, (int(x1), int(y2)), 3 ,(0, 255, 255), 2)
			cv2.circle(frame, center, 5, (0, 0, 255), -1)
			# update the points queue
			pts.appendleft(center2)
			if x1 > height/2 :
#				print >>rosCamr, 'down' 			
			elif x1 < height/2:
#				print >>rosCamr, 'up'
			if y2 > (width*2/3):
				ser.write('RM499\r\n')
			elif y2 < (width/3):
				ser.write('RM400\r\n')
#				joy_new.axes[0]=1.0

	if len(cnts) > 0:
		# find the largest contour in the mask, then use
		# it to compute the minimum enclosing circle and
		# centroid
		c = max(cnts, key=cv2.contourArea)
		((x, y), radius) = cv2.minEnclosingCircle(c)
		M = cv2.moments(c)
		center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
               	#start again
		if radius > 10:
			
			cv2.circle(frame, (int(x), int(y)), 3 ,(255, 255, 255), 2)
                	        # update the points queue
			pts.appendleft(center)
			if int(center[0])<(width*2/3) and int(center[0]) > (width/3) and int(center[1])<(height*2/3) and int(center[1])>(height/3):
				print >>rosWheelg, 'GO'
			if x > height/2 :
				print >>rosCamg, 'down'
			elif x < height/2:
				print >>rosCamg, 'up'
			if y > width/2:
				print >>rosWheelg, 'rigth'
			elif y < width/2:
				print >>rosWheelg, 'left'
      
	# loop over the set of tracked points
	for i in xrange(1, len(pts)):
		# if either of the tracked points are None, ignore
		# them
		if pts[i - 1] is None or pts[i] is None:
			continue

		# otherwise, compute the thickness of the line and
		# draw the connecting lines
		thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)

	# show the frame to our screen
	cv2.imshow("Frame", frame)
	key = cv2.waitKey(1) & 0xFF

	# if the 'q' key is pressed, stop the loop
	if   key == ord("q"):
		break
	elif key == ord("g"):
		goGreen = 1
		goRed = 0
	elif key == ord("r"):
		goGreen = 0
		goRed = 1

# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()



