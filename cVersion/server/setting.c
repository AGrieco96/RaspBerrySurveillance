#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static int xioctl(int fd, int request, void *arg){
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}


int changeResolution(int width,int height){
	    int fd;
 		//Open the capture device
        fd = open("/dev/video0", O_RDWR);
        if (fd == -1){
                perror("Opening video device");
                return 1;
        }

        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
        //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        
        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){
            perror("Setting Pixel Format");
            return 1;
        }
        close(fd);
        return 0;
}

int changeBrightness(int brightness_value){
	    int fd;
 		//Open the capture device
        fd = open("/dev/video0", O_RDWR);
        if (fd == -1){
                perror("Opening video device");
                return 1;
        }

	    struct v4l2_control setb;
        setb.id = V4L2_CID_BRIGHTNESS;
        setb.value = brightness_value;
        if( -1 == xioctl(fd,VIDIOC_S_CTRL,&setb)){
        	perror("Setting Exposure Erro");
        	return 1;
        }
        close(fd);
        return 0;
}

int changeContrast(int contrast_value){
	    int fd;
 		//Open the capture device
        fd = open("/dev/video0", O_RDWR);
        if (fd == -1){
                perror("Opening video device");
                return 1;
        }

		struct v4l2_control setc;
        setc.id = V4L2_CID_CONTRAST;
        setc.value = contrast_value;
        if( -1 == xioctl(fd,VIDIOC_S_CTRL,&setc)){
        	perror("Setting Exposure Erro");
        	return 1;
        }
        
        close(fd);
        return 0;
}

int changeExposure(int exposure_value){
		int fd;
		fd = open("/dev/video0", O_RDWR);
        if (fd == -1){
                perror("Opening video device");
                return 1;
        }
        //prima settiamo che l'exposure ce la modifichiamo noi
	    struct v4l2_control sete;
        sete.id = V4L2_CID_EXPOSURE_AUTO;
        sete.value = V4L2_EXPOSURE_MANUAL;
        if( -1 == xioctl(fd,VIDIOC_S_CTRL,&sete)){
        	perror("Setting Exposure Erro");
        	return 1;
        }
        
        //e poi vi settiamo il valore!
        struct v4l2_control sete_absolute;
        sete_absolute.id = V4L2_CID_EXPOSURE_ABSOLUTE;
        sete_absolute.value = exposure_value;
        if( -1 == xioctl(fd,VIDIOC_S_CTRL,&sete)){
        	perror("Setting Exposure Erro");
        	return 1;
        }
        
        close(fd);
        return 0;
}