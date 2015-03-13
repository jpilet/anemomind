# About
This subdirectory contains the code that will run on the
Anemobox and provide bluetooth services for data synchronization
with some other devices, such as a mobile phone.

This code makes the device a bluetooth peripheral
and uses the bleno library.

# Installation
These instructions assume you are running Ubilinux on your
Anemobox. In order to install Ubilinux, follow the instructions
here:

https://learn.sparkfun.com/tutorials/loading-debian-ubilinux-on-the-edison

Of course, since this code is based on node, you will 
first have to install node. I installed node on Ubilinux using
the instructions found here:

http://www.hostingadvice.com/how-to/install-nodejs-ubuntu-14-04/#maintained-ubuntu-packages

Also make sure that the node package manager, ```npm``` is installed.

Please go to the bleno Github page and read about bleno.

https://github.com/sandeepmistry/bleno

and install first the required libraries as explained
on the web page.

I ran these commands:

```
sudo apt-get install bluetooth bluez-utils libbluetooth-dev
sudo apt-get install libusb-1.0-0-dev
```

Finally, type
```
npm install
```
from the directory where this readme file resides.