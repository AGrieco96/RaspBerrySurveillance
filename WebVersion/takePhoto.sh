#!/bin/sh
NOME=`date "+%Y-%m-%d_%H-%M-%S"`
service motion stop
raspistill -o /var/www/html/files/Images/${NOME}.jpg
service motion start
