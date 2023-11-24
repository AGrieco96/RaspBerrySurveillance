#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include "takePhoto.h"
#include "poll.h"

void* polling(void* arg){
	sem_t* Rasputin=(sem_t*)arg;
	while(1){
		sem_wait(Rasputin);
        takePhoto();
	    fprintf(stderr , "Thread ceeeeeeeeeeeeeeeee");
        sem_post(Rasputin);
        sleep(10);
    }
}
