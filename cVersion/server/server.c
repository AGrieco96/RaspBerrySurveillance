#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>     //per i segnali
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <semaphore.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>

#include "server.h"
#include "setting.h"
#include "takePhoto.h"
#include "poll.h"

#include "setangle.h"
//#include "setangle.c"

//Semaphore
sem_t Rasputin;

//struct thread arg
typedef struct handler_args_s{
    int socket_desc;
    struct sockaddr_in *client_addr;
}    handler_args_t;


//control input function
int controlInput(char* buf , int* value){
	char* end;
	int n;
	errno = 0;
	const long sl = strtol(buf , &end , 10);

	
	if (end == buf) {
		if(DEBUG)
    		fprintf(stderr, "%s: not a decimal number\n", buf);
  	}else if ('\0' != *end) {
    	if(DEBUG)
    		fprintf(stderr, "%s: extra characters at end of input: %s\n", buf, end);
  	}else if ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) {
    	if(DEBUG)
    		fprintf(stderr, "%s out of range of type long\n", buf);
  	}else if (sl > INT_MAX) {
    	if(DEBUG)
    		fprintf(stderr, "%ld greater than INT_MAX\n", sl);
  	}else if (sl < INT_MIN) {
    	if(DEBUG)
    		fprintf(stderr, "%ld less than INT_MIN\n", sl);
  	}else {
    	*value = (int)sl;
    	return TRUE;
    }
    return FALSE;
}

//thread function
void *connection_handler(void *arg){
	int ret;
	int i;
	int index;
    int recv_bytes;
    int angle;
    char* resolution;
    int height;
    int widht;
    int brightness;
    int exposure;
    int contrast;
    //thread arg
    handler_args_t *args = (handler_args_t *)arg;        
    int socket_desc = args->socket_desc;
    struct sockaddr_in *client_addr = args->client_addr;
    //parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(client_addr->sin_port); 
    //local variables
    char buf[1024];
    size_t buf_len = sizeof(buf);
    char aux_buf[8];
    size_t aux_buf_len = sizeof(aux_buf);
    size_t msg_len;
    char path[256];
    size_t path_len =sizeof(path);
    char *quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);
    //control
    int control;
    //picture
    FILE *picture;
    char* send_buffer;
    int size;
    int read_bytes;
    int send_bytes;
    int aux;
    //directory
    DIR *dir;
    struct dirent *ent;
    //initialize welcome message
    memset(buf , 0 , buf_len);
    if(DEBUG) 
    	fprintf(stderr, "buf: %s\n", buf);
    sprintf(buf , WELCOME_MESS);
    msg_len = strlen(buf);
    if(DEBUG) 
    	fprintf(stderr, "buf: %s\n", buf);
    //send welcome message
    while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        if (errno == EINTR) continue;
        ret = close(socket_desc);
        //free buffer
    	free(args->client_addr); 
    	free(args);
    	//exit
    	pthread_exit(NULL);
    }
    //main loop
    while(1){
    	control = TRUE;
    	if(DEBUG)
    		fprintf(stderr, "Waiting options...\n");
    	//clear buf
    	memset(buf , 0 , buf_len);
    	if(DEBUG) 
    		fprintf(stderr, "buf: %s\n", buf);
    	//receive option from client
    	while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            if (errno == EINTR) continue;
            ret = close(socket_desc);
        	//free buffer
    		free(args->client_addr); 
    		free(args);
    		//exit
    		pthread_exit(NULL);
        }
        buf[recv_bytes] = '\0';
        if(DEBUG) 
    		fprintf(stderr, "buf: %s\n", buf);
        // check whether I have just been told to quit...
        if (recv_bytes == 0) break;
        if (recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;
        /********************************
        *								*
        *			SETANGLE			*
        *								*
        *********************************/
        if (recv_bytes == ANGLE_LEN && !memcmp(buf,ANGLE,ANGLE_LEN)) {
        	//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//inizialize buf
    		sprintf(buf , ANGLE_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//send angle mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//receive angle from client
    		while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
            	ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
        	}
        	buf[recv_bytes] = '\0';
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//input control
    		control = controlInput(buf , &angle);
    		if(control && DEBUG)
    			fprintf(stderr, "angle: %d\n", angle);
    		//range control
    		if(control && (angle < 0 || angle > 180))
    			control = FALSE;
    		//
    		if(control){
    			//wait on semaphore
    		    ret = sem_wait(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            if(DEBUG)
	            	fprintf(stderr , "ON Semaphore\n");
	            /************************
	            *                       *
	            *   SetAngle function   *
	            *                       *
	            ************************/
				setAngle(angle);

	            //post on semaphore
	            ret = sem_post(&Rasputin);
	            ERROR_HELPER(ret, "Wait on semaphore Rasputin");
    		}
            //clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//initialize welldone/nogood mess
    		if(control)
    			sprintf(buf , WELLDONE_MESS);
    		else
    			sprintf(buf , NOGOOD_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//send weel done mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		control = TRUE;
        }		//set angle done
        /********************************
        *								*
        *			  PHOTO				*
        *								*
        *********************************/
        else if (recv_bytes == PHOTO_LEN && !memcmp(buf,PHOTO,PHOTO_LEN)){
NF:        	//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//initilize photo mess
        	if(control){
        		sprintf(buf , PHOTO_MESS);
        		msg_len=strlen(buf);
        	}
        	else{
        		sprintf(buf , PHOTONOTFOUND_MESS);
        		msg_len = strlen(buf);
        	}
        	control = TRUE;
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//directory scanner
            if ((dir = opendir("files/")) != NULL) {
                while ((ent = readdir (dir)) != NULL) {
                    if ( !memcmp(ent->d_name,"..",1) || !memcmp(ent->d_name,".",2)) continue;   //skip '.' and '..' directories
                    sprintf(&buf[msg_len],"%s\n", ent->d_name);
                    msg_len=strlen(buf);
                }
            }else{  //cannot show directory
                ERROR_HELPER(-1, "Cannot show directory");
            }
            if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//send photo mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//receive file name
    		while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
                puts("chiudo connesione");
            	ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
        	}
        	buf[recv_bytes] = '\0';
            if(memcmp(buf,'\0',1) == 0) goto NF;
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//clear file path
    		memset(path , 0 , path_len);
    		if(DEBUG)
    			fprintf(stderr, "path:%s\n", path);
    		//create file path
            sprintf(path,"files/%s",buf);
            if(DEBUG)
    			fprintf(stderr, "path:%s\n", path);
    		
            //open file
    		if ((picture = fopen(path, "r")) != NULL){	//fhoto found
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
                //inizialize found message
                sprintf(buf,FOUND);
                msg_len=FOUND_LEN;
                if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
                //send found message
                while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			//wait on semaphore
            	ret = sem_wait(&Rasputin);
            	ERROR_HELPER(ret, "Wait on semaphore Rasputin");
            	if(DEBUG)
            		fprintf(stderr , "ON Semaphore\n");
            	//ready message
            	//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//receive file name
    			while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            		if (errno == EINTR) continue;
            		ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
        		}
        		buf[recv_bytes] = '\0';
        		if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
                //get picture size
                if(DEBUG)
                	fprintf(stderr , "Getting Picture Size\n");  
                fseek(picture, 0, SEEK_END);
                size = ftell(picture);
                fseek(picture,0, SEEK_SET);
                if(DEBUG)
                	fprintf(stderr, "size: %d\n", size);
                //send picture size
                if(DEBUG)
                	fprintf(stderr , "Sending Picture Size\n");
                while((ret = write(socket_desc, &size, sizeof(int))) < 0){
                	if(errno == EINTR) continue;
                	ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
                }
                //send picture as byte array
                if(DEBUG)
                	fprintf(stderr , "Sending Picture as Byte Array\n");   
                //write picture on array        
                send_buffer = (char*)malloc(sizeof(char)*size);
                read_bytes = 0;
                aux = size;
                do{
                	ret = fread(send_buffer+read_bytes , 1 , aux , picture);
                	aux-=ret;
                }while(aux > 0);
                //send array
                send_bytes = 0;
                aux = size;
                do{
                	ret = write(socket_desc , send_buffer+send_bytes , aux);
                	if(ret == -1 && errno == EINTR)	continue;
                	else if(ret==-1){
                		ret = close(socket_desc);
        				//free buffer
    					free(args->client_addr); 
    					free(args);
    					//exit
    					pthread_exit(NULL);
                	}
                	aux-=ret;
                }while(aux > 0);
                //free picture array
                free(send_buffer);
                //post on semaphore
            	ret = sem_post(&Rasputin);
            	ERROR_HELPER(ret, "Wait on semaphore Rasputin");
            }else{							//photo not found
            	//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//initialize not found mess
    			sprintf(buf , NOT_FOUND);
    			msg_len = NOT_FOUND_LEN;
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//send not found mess
    			while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			control=FALSE;
                goto NF;
            }
        }		//photo done
        /********************************
        *								*
        *			SETTINGS			*
        *								*
        *********************************/
        else if (recv_bytes == SETTINGS_LEN && !memcmp(buf,SETTINGS,SETTINGS_LEN)) {
        	//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//initialize settings mess
    		sprintf(buf , SETTINGS_MESS);
    		msg_len = strlen(buf);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//send settings message
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
    		//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//receive settings option from client
    		while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            	if (errno == EINTR) continue;
            	ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
        	}
        	buf[recv_bytes] = '\0';
        	if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//IF resolution
    		if (recv_bytes == RESOLUTION_LEN && !memcmp(buf,RESOLUTION,RESOLUTION_LEN)) {
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//initialize resolution mess
    			sprintf(buf , RESOLUTION_MESS);
    			msg_len = strlen(buf);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//send resolution mess
    			while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//receive resolution value
    			while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            		if (errno == EINTR) continue;
           			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
        		}
        		if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);

    			control = FALSE;
    			//check x presence
    			for (i = 0; buf[i] ; ++i){
    				if(buf[i] == 'x'){
    					index = i;
    					control = TRUE;
    				}
    			}
                //checking permitted resolution
                if (control && memcmp(buf,FULL_HD,FULL_HD_LEN) && memcmp(buf , STANDARD , STANDARD_LEN) && memcmp(buf , LOW_SD , LOW_SD_LEN))
                    control = FALSE;  
    			//clear aux_buf
    			memset(aux_buf , 0 , aux_buf_len);
    			if(DEBUG)
    				fprintf(stderr, "aux_buf: %s\n", aux_buf);
    			//take widht
    			if(control){
    				for(i = 0 ; i < index ; i++)
    					aux_buf[i] = buf[i];
    				control = controlInput(aux_buf , &widht);
    			}
    			if(control && DEBUG)
    				fprintf(stderr, "widht: %d\n", widht);
    			//clear aux_buf
    			memset(aux_buf , 0 , aux_buf_len);
    			if(DEBUG)
    				fprintf(stderr, "aux_buf: %s\n", aux_buf);
    			//take height
    			if(control){
    				for(i = index+1 ; buf[i] ; i++)
    					aux_buf[i-index-1] = buf[i];
    				control = controlInput(aux_buf ,&height);
    			}
    			if(control && DEBUG)
    				fprintf(stderr, "height: %d\n", height);

    			if(control){
	    			//.....Entro nel semaforo rasputin e cambio la risoluzione in mutua esclusione *****
	                ret = sem_wait(&Rasputin);
	                ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            	/********************************
	            	*                       		*
	            	*   changeResolution function	*
	            	*                       		*
	            	*********************************/
                    changeResolution(widht , height);
	                ret = sem_post(&Rasputin);
	                ERROR_HELPER(ret,"Exiting from semaphore Rasputin");
	            }

    		}	//resolution done
    		//IF brightness
            else if (recv_bytes == BRIGHTNESS_LEN && !memcmp(buf,BRIGHTNESS,BRIGHTNESS_LEN)) {
            	//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//initialize brightness mess
    			sprintf(buf , BRIGHTNESS_MESS);
    			msg_len = strlen(buf);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//send brightness mess
    			while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//receive brightness value
    			while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            		if (errno == EINTR) continue;
           			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
        		}
        		if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//input control
    			control = controlInput(buf , &brightness);
    			if(control && DEBUG)
    				fprintf(stderr, "brightness: %d\n", brightness);
    			if(control && (brightness < 1 || brightness > 255))
    				control = FALSE;
    			if(control){
	    			//.....Entro nel semaforo rasputin e cambio la risoluzione in mutua esclusione *****
	                ret = sem_wait(&Rasputin);
	                ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            	/********************************
	            	*                       		*
	            	*   changeBrightness function	*
	            	*                       		*
	            	*********************************/
	            	changeBrightness(brightness);
	                ret = sem_post(&Rasputin);
	                ERROR_HELPER(ret,"Exiting from semaphore Rasputin");
	            }

            }//brightness done
            //IF exposure
            else if (recv_bytes == EXPOSURE_LEN && !memcmp(buf,EXPOSURE,EXPOSURE_LEN)) {
            	//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//initialize exposure mess
    			sprintf(buf , EXPOSURE_MESS);
    			msg_len = strlen(buf);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//send exposure mess
    			while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//receive exposure value
    			while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            		if (errno == EINTR) continue;
           			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
        		}
        		if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			control = controlInput(buf , &exposure);
    			if(control && DEBUG)
    				fprintf(stderr, "exposure: %d\n", exposure);
    			if(control && (exposure < 1 || exposure > 255))
    				control = FALSE;
    			if(control){
	    			//.....Entro nel semaforo rasputin e cambio la risoluzione in mutua esclusione *****
	                ret = sem_wait(&Rasputin);
	                ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            	/********************************
	            	*                       		*
	            	*   changeExposure function	*
	            	*                       		*
	            	*********************************/
	            	changeExposure(exposure);
	                ret = sem_post(&Rasputin);
	                ERROR_HELPER(ret,"Exiting from semaphore Rasputin");
	            }
            }//exposure done
            //IF contrast
            if (recv_bytes == CONTRAST_LEN && !memcmp(buf,CONTRAST,CONTRAST_LEN)) {
            	//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//initialize contrast mess
    			sprintf(buf , CONTRAST_MESS);
    			msg_len = strlen(buf);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//send contrast mess
    			while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        			if (errno == EINTR) continue;
        			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
    			}
    			//clear buf
    			memset(buf , 0 , buf_len);
    			if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			//receive contrast value
    			while ((recv_bytes = recv(socket_desc, buf, buf_len, 0)) < 0){
            		if (errno == EINTR) continue;
           			ret = close(socket_desc);
        			//free buffer
    				free(args->client_addr); 
    				free(args);
    				//exit
    				pthread_exit(NULL);
        		}
        		if(DEBUG) 
    				fprintf(stderr, "buf: %s\n", buf);
    			control = controlInput(buf , &contrast);
    			if(control && DEBUG)
    				fprintf(stderr, "contrast: %d\n", contrast);
    			if(control &&(contrast < 1 || contrast > 255))
    				control = FALSE;
    			if(control){
	    			//.....Entro nel semaforo rasputin e cambio la risoluzione in mutua esclusione *****
	                ret = sem_wait(&Rasputin);
	                ERROR_HELPER(ret, "Wait on semaphore Rasputin");
	            	/********************************
	            	*                       		*
	            	*   changeContrast function	    *
	            	*                       		*
	            	*********************************/
                    changeContrast(contrast);
	                ret = sem_post(&Rasputin);
	                ERROR_HELPER(ret,"Exiting from semaphore Rasputin");
	            }
            }
    		//clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG) 
    			fprintf(stderr, "buf: %s\n", buf);
    		//initialize welldone/nogood mess
    		if(control)
    			sprintf(buf , WELLDONE_MESS);
    		else
    			sprintf(buf , NOGOOD_MESS);
    		msg_len = strlen(buf);
    		//send well done mess
    		while ((ret = send(socket_desc, buf, msg_len, 0)) < 0){
        		if (errno == EINTR) continue;
        		ret = close(socket_desc);
        		//free buffer
    			free(args->client_addr); 
    			free(args);
    			//exit
    			pthread_exit(NULL);
    		}
        }//settings done
    }		//mainloop
    // close socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");
    if (DEBUG)
        fprintf(stderr, "Thread created to handle the request has completed.\n");
    //free buffer
    free(args->client_addr); 
    free(args);
    //exit
    pthread_exit(NULL);
}

//main function
int main(int argc, char *argv[]){
    int ret;
    //semaphore initialized
    ret=sem_init(&Rasputin,0,1);
    ERROR_HELPER(ret,"Failed to inizialize semaphore");
    //Thread Polling
    pthread_t poll_thread;
    ret=pthread_create(&poll_thread,NULL,polling,&Rasputin);
    PTHREAD_ERROR_HELPER(ret,"Could not create thread");
    ret=pthread_detach(poll_thread);
    PTHREAD_ERROR_HELPER(ret , "Could not detach thread");
    // End Polling
    int socket_desc, client_desc;
    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0};
    int sockaddr_len = sizeof(struct sockaddr_in); // we will reuse it for accept()
    // initialize socket for listening
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); // don't forget about network byte order!

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr *)&server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    // we allocate client_addr dynamically and initialize it to zero
    struct sockaddr_in *client_addr = calloc(1, sizeof(struct sockaddr_in));

    // loop to manage incoming connections spawning handler threads
    while (1){
        // accept incoming connection
        client_desc = accept(socket_desc, (struct sockaddr *)client_addr, (socklen_t *)&sockaddr_len);
        if (client_desc == -1 && errno == EINTR)
            continue; // check for interruption by signals
        ERROR_HELPER(client_desc, "Cannot open socket for incoming connection");

        if (DEBUG)
            fprintf(stderr, "Incoming connection accepted...\n");

        pthread_t thread;

        // put arguments for the new thread into a buffer
        handler_args_t *thread_args = malloc(sizeof(handler_args_t));
        thread_args->socket_desc = client_desc;
        thread_args->client_addr = client_addr;

        ret = pthread_create(&thread, NULL, connection_handler, (void *)thread_args);
        PTHREAD_ERROR_HELPER(ret, "Could not create a new thread");

        if (DEBUG)
            fprintf(stderr, "New thread created to handle the request!\n");

        ret = pthread_detach(thread); // I won't phtread_join() on this thread
        PTHREAD_ERROR_HELPER(ret, "Could not detach the thread");
        // we can't just reset fields: we need a new buffer for client_addr!
        client_addr = calloc(1, sizeof(struct sockaddr_in));
    }

    exit(EXIT_SUCCESS); // this will never be executed
}
