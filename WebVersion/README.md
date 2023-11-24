# Raspeberry Pi 3 Home Video Surveillance
<br>
---
- **Products:**
    - Raspberry Pi 3 model B
    - Raspberry Pi official camera module v2
    - Servo motor 5V
    - Micro SD
    - Ethernet cable
    - Optional:
    	- Raspberry Pi case 
    	- Camera case
    	- Cooling fan
    	- Heat sink

---
<br>

## Install and setting up Raspbian

#### Install

You should install the OS to the Raspberry Pi , we have chosen Raspbian ([you can download this from this link](https://www.raspberrypi.org/downloads)).
<br>
Extract it and you should have an image file (.img) that contains the operating system.
<br>
We have used [Win32 Disk Manager](https://sourceforge.net/projects/win32diskimager) (available only on Windows) to write the OS into the SD card.
<br><br>
**If you're using a version of Raspbian relelased after november 2016** SSH is disabled by default, so you need to creat a file called *SSH* and place it in the root partition of the SD card. now you can eject the SD from your computer.

#### Setup

Insert the SD card into the Raspberry Pi and power on it. Plug the Raspberry to your router within the ethernet cable, in that way you can acces to your Raspberry Pi trought SSH.

* If you are using Windows you need to install [Putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html) on your PC, that  establishes an SSH connection and gives you access to the Raspberry Pi’s command prompt.<br>
* If you are using Mac or Linux, you can SSH into the Pi with Terminal.

In order to find the Pi's IP address you can check the router’s configuration page.
<br>
Now you can acces to your Pi trougth SSH to log enter *pi* for the username, and *raspberry* as the password.<br>
At this point you can configure your Raspberry Pi entering 'sudo raspi-config' to enter the configuration setting menu.
This is where you can change all the default setting of your Pi and do other funny stuff.

## Setting up WiFi