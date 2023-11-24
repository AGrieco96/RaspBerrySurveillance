#ifndef TAKEPHOTO_H
	#define TAKEPHOTO_H
	
	static int xioctl(int fd, int request, void *arg);
	int print_caps(int fd);
	int init_mmap(int fd);
	int capture_image(int fd);
	int takePhoto();
#endif