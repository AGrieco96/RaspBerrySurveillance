#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#include "client.h"

int control = FALSE;
int socket_desc;
int fd;

void signal_handler(int sig){
    int ret;
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG){ 
        ret = close(fd);
        ERROR_HELPER(ret , "Could not close error.log");
    }
    printf("Exiting...\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]){
	int ret;
	

    (void) signal(SIGINT, signal_handler);

	if(DEBUG){
		fd = open("error.log" , O_CREAT|O_WRONLY|O_TRUNC);
		ERROR_HELPER(fd , "Could not create error.log");
		ret = dup2(fd,2);
		ERROR_HELPER(ret , "Could not redirect stderr");
	}
    // variables for handling a socket

    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0
    // create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");
    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!
    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Could not create connection");
    if (DEBUG) fprintf(stderr, "Connection established!\n");
    //useful variables
    char buf[1024];
    size_t buf_len = sizeof(buf);
    size_t msg_len;
    char path[126];
    size_t path_len = sizeof(path);
    int i;
    //quit command
    char* quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);
    //picture
    int size;
    FILE *image;
    char* p_array;
    int recv_bytes;
    int written_bytes;
    //clear buf
    memset(buf , 0 , buf_len);
    if(DEBUG)
    	fprintf(stderr, "buf: %s\n", buf);
    //receive welcome message from server
    while ( (msg_len = recv(socket_desc, buf, buf_len - 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot read from socket");
    }
    buf[msg_len] = '\0';
    //display welcome message
    printf("%s", buf);
    //main loop
    while(1){
    	do{ 
    		//printf option
            if(control)
                printf("Ops... \nInsert valid option, please.\n");
            control = FALSE;
            printf("-SearchPhoto\n-SetAngle\n-Settings\n-Quit (DO NOT DO IT)\nInsert option: ");
            //clear buf
    		memset(buf , 0 , buf_len);
    		if(DEBUG)
    			fprintf(stderr, "buf: %s\n", buf);
    		//read from stdin
    		if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }
            msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //to lower case
            for ( i = 0; i < msg_len; ++i){
            	buf[i] = tolower(buf[i]);
            }
            control = TRUE;
        }while(!(msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len))  &&  //not QUIT
                    !(msg_len == PHOTO_LEN && !memcmp(buf, PHOTO, PHOTO_LEN))                       &&  //not PHOTO
                    !(msg_len == SETTINGS_LEN && !memcmp(buf, SETTINGS, SETTINGS_LEN))              &&  //not SETTINGS
                    !(msg_len == ANGLE_LEN && !memcmp(buf, ANGLE, ANGLE_LEN))                           //not ANGLE
                );		//control right option
        control = FALSE;
        //send option to server
        while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot write to socket");
        }
        //quit control
        if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;
        //IF photo
        else if (msg_len == PHOTO_LEN && !memcmp(buf, PHOTO, PHOTO_LEN)){
        	do{
        		//clear buf
        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);
        		//receive photos list
            	while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                    if (errno == EINTR) continue;
                    ERROR_HELPER(-1, "Cannot read from socket");
                }
                buf[msg_len] = '\0';
                //display photos list
                printf("\n%s", buf); // no need to insert '\0'
                //clear buf
        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);
        		printf("Choose photo: ");
            	if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                	fprintf(stderr, "Error while reading from stdin, exiting...\n");
                	exit(EXIT_FAILURE);
            	}
            	msg_len = strlen(buf);
            	buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            	if(DEBUG)
            		fprintf(stderr, "buf: %s\n", buf);
            	//clear path
            	memset(path , 0 , path_len);
            	if(DEBUG)
            		fprintf(stderr, "path: %s\n", path);
            	//put filename in path variable
            	sprintf(path , "files/%s" , buf);
            	if(DEBUG)
            		fprintf(stderr, "path: %s\n", path);
            	//send file name
            	while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
            		if (errno == EINTR) continue;
            		ERROR_HELPER(-1, "Cannot write to socket");
        		}
        		//clear buf
        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);
        		//waiting for found/not found
            	while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                	if (errno == EINTR) continue;
                	ERROR_HELPER(-1, "Cannot read from socket");
            	}
            	buf[msg_len] = '\0';
            	if(DEBUG)
            		fprintf(stderr, "buf: %s\n", buf);
        	}while(!(msg_len == FOUND_LEN && !memcmp(buf , FOUND , FOUND_LEN)));
        	//photo found
            //ready mess
            //clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
                fprintf(stderr, "buf: %s\n", buf);
            //initialize ready mess
            sprintf(buf , "ready");
            msg_len = strlen(buf);
            if(DEBUG)
                fprintf(stderr, "buf: %s\n", buf);
            //send ready to server
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }
        	//read picture size
        	if(DEBUG)
				fprintf(stderr , "Reading Picture Size\n");
			while((ret = read(socket_desc, &size, sizeof(int))) < 0){
                if(errno == EINTR) continue;
                ERROR_HELPER(-1  , "Could not receive photo size");
            }
            if(DEBUG)
            	fprintf(stderr, "size: %d\n", size);
			//read picture byte array
			if(DEBUG)
				fprintf(stderr , "Reading Picture Byte Array\n");
            p_array = (char*)malloc(sizeof(char)*size);
            recv_bytes = 0;
            do{
                ret = read(socket_desc, p_array+recv_bytes, size);
                if(ret == -1 && errno == EINTR)  continue;
                else if(ret == -1)               ERROR_HELPER(ret , "could not transfer the file");
                recv_bytes+=ret;
            }while(recv_bytes < size);
			//convert it back into picture
			if(DEBUG)
				fprintf(stderr , "Converting Byte Array to Picture\n");	
			image = fopen(path, "w");
			if(DEBUG)
				fprintf(stderr , "Picture opened\n");	
            written_bytes = 0;
			do{
                ret = fwrite(p_array+written_bytes, 1, size, image);
                size -= ret;
            }while(size > 0);
			if(DEBUG)
				fprintf(stderr , "Converted Byte Array to Picture\n");	
			fclose(image);
            free(p_array);
        }//photo done
        //IF settings
        else if (msg_len == SETTINGS_LEN && !memcmp(buf, SETTINGS, SETTINGS_LEN)){
        	do{
        		if(control)
        			printf(SETTINGS_MESS);
        		else{
        			//clear buf
        			memset(buf , 0 , buf_len);
        			if(DEBUG)
        				fprintf(stderr, "buf: %s\n", buf);
        			//Print setting list
            		while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                		if (errno == EINTR) continue;
                		ERROR_HELPER(-1, "Cannot read from socket");
            		}
            		printf("%s", buf); // no need to insert '\0'
            	}
            	//clear buf
        		memset(buf , 0 , buf_len);
        		if(DEBUG)
        			fprintf(stderr, "buf: %s\n", buf);
        		//insert setting option
            	if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                	fprintf(stderr, "Error while reading from stdin, exiting...\n");
                	exit(EXIT_FAILURE);
            	}
            	msg_len = strlen(buf);
            	buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            	if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            	//to lower case
            	for( i = 0; buf[i]; i++){
                	buf[i] = tolower(buf[i]);
            	}
            	control = TRUE;
        	}while(!(msg_len == RESOLUTION_LEN && !memcmp(buf, RESOLUTION, quit_command_len))  		&&	//not RESOLUTION
                    !(msg_len == BRIGHTNESS_LEN && !memcmp(buf, BRIGHTNESS, BRIGHTNESS_LEN))    &&	//not BRIGHTNESS
                    !(msg_len == EXPOSURE_LEN && !memcmp(buf, EXPOSURE, EXPOSURE_LEN))          &&	//not EXPOSURE
                    !(msg_len == CONTRAST_LEN && !memcmp(buf, CONTRAST, CONTRAST_LEN))            	//not CONTRAST
                );
        	control = FALSE;
        	//send settings option
        	while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }
            //clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //receive server message
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s", buf); // no need to insert '\0'
            //clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //insert value
            if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }
			msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //send value
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
           	    ERROR_HELPER(-1, "Cannot write to socket");
            }
			//clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //waiting for "well done" message
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s", buf); // no need to insert '\0'
		}//settings done
		//if ANGLE
        else if (msg_len == ANGLE_LEN && !memcmp(buf, ANGLE, ANGLE_LEN)){
        	//clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //receive angle mess
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s", buf); // no need to insert '\0'
            //clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            //insert angle
            memset(buf , 0 , buf_len);
            if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
                fprintf(stderr, "Error while reading from stdin, exiting...\n");
                exit(EXIT_FAILURE);
            }
            msg_len = strlen(buf);
            buf[--msg_len] = '\0'; // remove '\n' from the end of the message
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            // send angle to server
            while ( (ret = send(socket_desc, buf, msg_len, 0)) < 0) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot write to socket");
            }
            //clear buf
            memset(buf , 0 , buf_len);
            if(DEBUG)
            	fprintf(stderr, "buf: %s\n", buf);
            // waiting for "well done" message
            memset(buf , 0 , buf_len);
            while ( (msg_len = recv(socket_desc, buf, buf_len, 0)) < 0 ) {
                if (errno == EINTR) continue;
                ERROR_HELPER(-1, "Cannot read from socket");
            }
            printf("%s \n", buf); // no need to insert '\0'
        }//angle done
    }
    // close the socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG){ 
    	ret = close(fd);
    	ERROR_HELPER(ret , "Could not close error.log");
	}

	printf("Exiting...\n");

    exit(EXIT_SUCCESS);
}