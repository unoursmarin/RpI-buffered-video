#!/bin/sh
clear # Get all the files of the folder /home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot

bash -c "exec -a CameraRecord ./raspberrypi_video &" #Chemin relatif 
echo "Press a+Enter to trigger alarm"

validate= false
while true
	do
	read key
		if [$key -eq ""];then
		
		 pkill -f CameraRecord
		 find /home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot -name '*' -exec ls -t {} \; | sort -k 5 -n > /tmp/screenshot.txt


		 image_name=$(sed -n 1p /tmp/screenshot.txt)
		 name=$(echo "$image_name"| cut -f 1 -d '.')
		 echo $name

		 # frames to video
		 cd /home/pi/Documents/LeptonModule-master/software/raspberrypi_video/screenshot
		 ffmpeg -framerate 30 -start_number $name -i %d.png video.avi
		
		fi
	done 
