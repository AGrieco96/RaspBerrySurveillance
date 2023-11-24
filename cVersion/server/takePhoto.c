#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include <time.h>

#include "takePhoto.h"

int BUFFER_LEN;

uint8_t *buffer;

//function to manipulate device parameters of special file.
static int xioctl(int fd, int request, void *arg){
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}

int print_caps(int fd){
        struct v4l2_capability caps = {};
        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &caps)){
                perror("Querying Capabilities");
                return 1;
        }
 
        printf( "Driver Caps:\n"
                "  Driver: \"%s\"\n"
                "  Card: \"%s\"\n"
                "  Bus: \"%s\"\n"
                "  Version: %d.%d\n"
                "  Capabilities: %08x\n",
                caps.driver,
                caps.card,
                caps.bus_info,
                (caps.version>>16)&&0xff,
                (caps.version>>24)&&0xff,
                caps.capabilities);
 

        struct v4l2_cropcap cropcap = {0};
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
 
        printf( "Camera Cropping:\n"
                "  Bounds: %dx%d+%d+%d\n"
                "  Default: %dx%d+%d+%d\n"
                "  Aspect: %d/%d\n",
                cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
                cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
                cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

/*
        int support_grbg10 = 0;
        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        char c, e;
        printf("  FMT : CE Desc\n--------------------\n");
        while (0 == xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
        {
                strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
                if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
                    support_grbg10 = 1;
                c = fmtdesc.flags & 1? 'C' : ' ';
                e = fmtdesc.flags & 2? 'E' : ' ';
                printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
                fmtdesc.index++;
        }


        //strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
        printf( "Selected Camera Mode:\n"
                "  Width: %d\n"
                "  Height: %d\n"
                "  PixFmt: NULL\n"          // come era prima PixFmt: %s\n
                "  Field: %d\n",
                fmt.fmt.pix.width,
                fmt.fmt.pix.height,         //fourcc
                fmt.fmt.pix.field);
        return 0;
        */
        return 0;
}

 
int init_mmap(int fd){
    //buffer that contains data exchanged by application and driver using Streaming I/O methods.
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    //used to initialize memory mapped (mmap)
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)){
        perror("Requesting Buffer");
        return 1;
    }
    //after requesting buffer from the device, we need to query the buffer in order to get raw data.
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)){
        perror("Querying Buffer");
        return 1;
    }
 
    buffer = (uint8_t *) mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    printf("Image Length: %d\n", buf.bytesused);
    BUFFER_LEN=buf.length;
    return 0;
}
 
int capture_image(int fd){
    
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
        perror("Query Buffer");
        return 1;
    }
    if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type)){
        perror("Start Capture");
        return 1;
    }
 
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r){
        perror("Waiting for Frame");
        return 1;
    }
 
    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf)){
        perror("Retrieving Frame");
        return 1;
    }
    printf ("saving image\n");
    
    /*
    IplImage* frame;
    CvMat cvmat = cvMat(480, 640, CV_8UC3, (void*)buffer);
    frame = cvDecodeImage(&cvmat, 1);
    cvNamedWindow("window",CV_WINDOW_AUTOSIZE);
    cvShowImage("window", frame);
    cvWaitKey(0);
	*/
    /*************************************************************
    Operazione per creare imaggini con nome secondo la data locale
    **************************************************************/
    time_t data;
    struct tm *leggibile = NULL;

    time(&data);
    leggibile = localtime(&data);
    char bufferino[32];
    
    sprintf(bufferino,"files/%d-%d-%d||%d:%d:%d.jpg",leggibile->tm_year+1900,
        leggibile->tm_mon+1,
        leggibile->tm_mday, 
        leggibile->tm_hour, 
        leggibile->tm_min,
        leggibile->tm_sec);

   //cvSaveImage(buffer, frame, 0);
 	
    //open files
 	int outfd = open (bufferino, O_RDWR | O_CREAT);
    if(outfd == -1){
        perror("Cannot create image");
        return 1;
    }
    //write image on file
    int written_bytes = 0;
    int ret;
    while(written_bytes < buf.bytesused){
        ret = write(outfd , buffer+written_bytes , buf.bytesused-written_bytes);
        if(ret == -1 && errno == EINTR)
            continue;
        if(ret == -1){
            perror("Failed to save image");
            return 1;
        }
        written_bytes += ret;
    }
 	//close file
 	ret = close(outfd);
    if(ret == -1){
        perror("Cannot close image");
        return 1;
    }

    return 0;
}
 
int takePhoto(){
        
	int fd;
        //Open the capture device
        fd = open("/dev/video0", O_RDWR);
        if (fd == -1)
        {
                perror("Opening video device");
                return 1;
        }
        //Check if the capture is avaiable
        if(print_caps(fd))
            return 1;
        

        if(init_mmap(fd))
            return 1;
        
	int ret;
	if(capture_image(fd))
            return 1;
    
        munmap(buffer,BUFFER_LEN);
	ret=close(fd);
	if (ret==-1) puts("Pene");
	
	
        return 0;

}
