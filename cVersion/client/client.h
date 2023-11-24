#ifndef CLIENT_H
	#define CLIENT_H

	// macro to simplify error handling
	#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {               \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)

	#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
	#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

	/* Configuration parameters */
	#define DEBUG           1   // display debug messages
	#define SERVER_ADDRESS  "192.168.1.101"
	#define SERVER_COMMAND  "quit"
	#define SERVER_PORT     2015

    #define PHOTO 			"searchphoto"
    #define ANGLE 			"setangle"
    #define SETTINGS		"settings"
    #define PHOTO_LEN 		strlen(PHOTO)
    #define ANGLE_LEN 		strlen(ANGLE)
    #define SETTINGS_LEN 	strlen(SETTINGS)

    #define RESOLUTION 		"resolution"
    #define EXPOSURE 		"exposure"
    #define BRIGHTNESS 		"brightness"
    #define CONTRAST 		"contrast"
    #define RESOLUTION_LEN 	strlen(RESOLUTION)
    #define EXPOSURE_LEN 	strlen(EXPOSURE) 
    #define BRIGHTNESS_LEN 	strlen(BRIGHTNESS)
    #define CONTRAST_LEN 	strlen(CONTRAST)

    #define FALSE 			0
    #define TRUE 			1

    #define FOUND 			"found"
    #define FOUND_LEN 		strlen(FOUND)
    #define NOT_FOUND		"notfound"
    #define NOT_FOUND_LEN	strlen(NOT_FOUND)

    #define SETTINGS_MESS   "Insert valid option.\nWhat do you want modify?\nResolution\nBrightness\nExposure\nContrast\n"

#endif
