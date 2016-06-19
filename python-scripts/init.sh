#!/bin/sh

# Déclanchement automatique : copier coller le texte ci-dessous dans nano /etc/cron.d/arduilink
# 
# SHELL=/bin/sh
# PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
# @reboot   root    /var/www/html/plugins/script/core/ressources/init.sh


sudo chmod 777 /dev/ttyUSB0

cd /var/www/html/plugins/script/core/ressources

sudo rm server.log
sudo rm client.log

sudo ./arduilink_server.py -p 1008 > ./server.log &

sleep 6

sudo ./arduilink_client.py -p 1008 -w 1 -r "php ./jeedom_push.php 13" > ./client.log &