CC = gcc
#C+ = g++
THREAD = -lpthread
#OPENCV = `pkg-config opencv --cflags --libs`
#LIBS = $(OPENCV)
#MIX = -lstdc++

all:		gpio soft_pwm event_gpio cpuinfo common server setangle settings poll takephoto final

gpio:		utility_servo/gpio.c utility_servo/gpio.h	
	$(CC) -c utility_servo/gpio.c 

soft_pwm: 	utility_servo/soft_pwm.c utility_servo/soft_pwm.h
	$(CC) -c utility_servo/soft_pwm.c

event_gpio: utility_servo/event_gpio.c utility_servo/event_gpio.h
	$(CC) -c utility_servo/event_gpio.c

cpuinfo: 	utility_servo/cpuinfo.c utility_servo/cpuinfo.h
	$(CC) -c utility_servo/cpuinfo.c

common: 	utility_servo/common.c utility_servo/common.h
	$(CC) -c utility_servo/common.c

server: 	server.c server.h
	$(CC) -c server.c

setangle:	setangle.c setangle.h
	$(CC) -c setangle.c

settings: 	setting.c setting.h
	$(CC) -c setting.c

takephoto: 	takePhoto.c takePhoto.h
	$(CC) -c takePhoto.c

poll:		poll.c poll.h
	$(CC) -c poll.c

final: 		
	$(CC) -o server server.o gpio.o soft_pwm.o event_gpio.o cpuinfo.o common.o setangle.o setting.o takePhoto.o poll.o $(THREAD)

.PHONY:clean

clean:
	rm -f server *.o utility_servo/*.o

run:
	./server

