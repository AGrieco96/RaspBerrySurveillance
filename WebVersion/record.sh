#!/bin/sh

NOME=`date "+%Y-%m-%d_%H-%M-%S"`
sudo service motion stop
raspivid -t 0 -o /var/www/html/files/movies/${NOME}.mp4

