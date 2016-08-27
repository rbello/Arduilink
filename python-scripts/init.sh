#!/bin/sh

# Déclanchement automatique : copier coller le texte ci-dessous dans nano /etc/cron.d/arduilink
# 
# SHELL=/bin/sh
# PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
# @reboot   root    /var/www/html/plugins/script/core/ressources/init.sh

cd /var/www/html/plugins/script/core/ressources

sudo chown jeedom:www-data client.log
sudo chown jeedom:www-data server.log
sudo chown jeedom:www-data arduilink_server.py
sudo chown jeedom:www-data arduilink_client.py

pathToArduino=/dev/ttyUSB0
if [ ! -f pathToArduino ]; then
	echo $pathToArduino
	pathToArduino=/dev/ttyACM0
    if [ ! -f pathToArduino ]; then
		echo $pathToArduino
		echo "Error: Arduino is not connected"
		echo "Error: Arduino is not connected" > ./server.log
		exit 2
	fi
fi
echo $pathToArduino
exit 0

sudo chmod 777 $pathToArduino

# Server startup
sudo ./arduilink_server.py --port 1008 --file $pathToArduino > ./server.log 2>&1 &

sleep 5

# Capteur EDF arduino #1 vers jeedom ID #13
sudo ./arduilink_client.py --port 1008 --mode watch --target 2:1 --run "php ./jeedom_push.php 13" > ./client.log 2>&1 &