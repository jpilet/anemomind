## Setting up the anemobox

# basics

1. Connect the anemobox to your computer using two micro usb cables.

2. Check your anemobox serial port with the command: `ls /dev/tty.*`

3. in my case, the port of the anemobox is `/dev/tty.usbserial-A9049MM3`
   connect to it using: 
   screen /dev/tty.usbserial-A9049MM3 115200
   ( to quit: ctrl-a then ctrl \ )

# Network: yocto

Run the setup tool to connect it to your wifi network
    `configure_edison --setup` I used the name anemobox.

Connect via ssh `ssh root@anemobox.local` using the password you specified.

# Network: debian

edit the file /etc/network/interfaces

mine looks like:
    
    #auto usb0
    iface usb0 inet static
        address 192.168.2.15
        netmask 255.255.255.0

    #auto wlan0
    iface wlan0 inet dhcp
        # For WPA
        wpa-ssid JulienNetworkN
        wpa-psk "password"
        # For WEP
        #wireless-essid Emutex
        #wireless-mode Managed
        #wireless-key s:password
    
run `ifup wlan0` to connect to the wireless network, then `ssh root@anemobox.local`.

    
# Software: Yocto
1. To install GIT insert these lines in `/etc/opkg/base-feeds.conf`:
```
src all     http://iotdk.intel.com/repos/1.1/iotdk/all
src x86 http://iotdk.intel.com/repos/1.1/iotdk/x86
src i586    http://iotdk.intel.com/repos/1.1/iotdk/i586
```
Then run:
```
opkg update && opkg install git
```

2. install node gyp and poco. You might have to add this path:
`export LD_LIBRARY_PATH=/usr/local/lib`


3. If you get this error message while doing `npm install`, try with this argument:
`npm install --unsafe-perm`

# Software: Debian

1. Install node by typing:
	```
	curl -sL https://deb.nodesource.com/setup_0.12 | sudo bash -
	sudo apt-get install -y nodejs
	```

2. type `apt-get update; apt-get upgrade; apt-get install git libpoco-dev libboost-thread-dev protobuf-compiler libprotobuf-dev libboost-iostreams-dev`

3. In the anemonode folder, type: `node-gyp configure; node-gyp build`
