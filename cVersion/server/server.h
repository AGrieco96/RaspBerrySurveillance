#ifndef SERVER_H
	#define SERVER_H

	// macro to simplify error handling
	#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {           \
        if (cond) {                                                 \
            fprintf(stderr, "%s: %s\n", msg, strerror(errCode));    \
            exit(EXIT_FAILURE);                                     \
        }                                                           \
    } while(0)

	#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
	#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

	/* Configuration parameters */
	#define DEBUG               1   // display debug messages
	#define SERVER_COMMAND      "quit"
	#define SERVER_PORT         2015
    #define MAX_CONN_QUEUE      5
    //boolean
    #define FALSE               0
    #define TRUE                1
    //server comands
    #define PHOTO               "searchphoto"
    #define ANGLE               "setangle"
    #define SETTINGS            "settings"
    #define PHOTO_LEN           strlen(PHOTO)
    #define ANGLE_LEN           strlen(ANGLE)
    #define SETTINGS_LEN        strlen(SETTINGS)
    //settings commands
    #define RESOLUTION          "resolution"
    #define EXPOSURE            "exposure"
    #define BRIGHTNESS          "brightness"
    #define CONTRAST            "contrast"
    #define RESOLUTION_LEN      strlen(RESOLUTION)
    #define EXPOSURE_LEN        strlen(EXPOSURE) 
    #define BRIGHTNESS_LEN      strlen(BRIGHTNESS)
    #define CONTRAST_LEN        strlen(CONTRAST)
    #define FULL_HD             "1920x1080"
    #define FULL_HD_LEN         strlen(FULL_HD)
    #define STANDARD            "1280x720"
    #define STANDARD_LEN        strlen(STANDARD)
    #define LOW_SD              "800x600"
    #define LOW_SD_LEN          strlen(LOW_SD)
    //photo commands
    #define FOUND               "found"
    #define FOUND_LEN           strlen(FOUND)
    #define NOT_FOUND           "notfound"
    #define NOT_FOUND_LEN       strlen(NOT_FOUND)
    //messages
    #define WELCOME_MESS        "Hi! I'm the GNM Server :)\nWhat do you want to do? I will stop if you send me quit \n"
    #define ANGLE_MESS          "Please give me the angle: "
    #define PHOTO_MESS          "Please choose a photo from the list belove:\n"
    #define PHOTONOTFOUND_MESS  "File not found,please chose a photo from the list below:\n"
    #define SETTINGS_MESS       "What do you want modify?\n-Resolution\n-Brightness\n-Exposure\n-Contrast\nInsert option: "
    #define RESOLUTION_MESS     "Please tell me the value you want to set resolution (ex: widhtxheight)\nAccepted:\n-1920x1080\n-1280x720\n-800x600\n Insert Option: "
    #define BRIGHTNESS_MESS     "Please tell me the value you want to set brightness (range 1-100): "
    #define EXPOSURE_MESS       "Please tell me the value you want to set exposure (range 1-1000): "
    #define CONTRAST_MESS       "Please tell me the value you want to set contrast (range 1-100): "
    #define WELLDONE_MESS       "Well Done\n"
    #define NOGOOD_MESS         "Invalid input!\n"

    int controlInput(char* buf , int* value);

#endif
