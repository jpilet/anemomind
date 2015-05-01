## Setting up the anemobox

1. Connect the anemobox to your computer using two micro usb cables.

2. Check your anemobox serial port with the command: `ls /dev/tty.*`

3. in my case, the port of the anemobox is `/dev/tty.usbserial-A9049MM3`

4. Run the setup tool to connect it to your wifi network
    `configure_edison --setup` I used the name anemobox.
    
5. Connect via ssh `ssh root@anemobox.local` using the password you specified.

6. To install GIT insert these lines in `/etc/opkg/base-feeds.conf`:
```
src all     http://iotdk.intel.com/repos/1.1/iotdk/all
src x86 http://iotdk.intel.com/repos/1.1/iotdk/x86
src i586    http://iotdk.intel.com/repos/1.1/iotdk/i586
```
Then run:
```
opkg update && opkg install git
```

7. install node gyp and poco. You might have to add this path:
`export LD_LIBRARY_PATH=/usr/local/lib`


9. install protobuf:
`sudo port install protobuf-cpp`

10. If you get this error message while doing `npm install`, try with this argument:
`npm install --unsafe-perm`