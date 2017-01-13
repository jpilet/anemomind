## How to update the firmware on the anemobox

0. Make sure your computer has access to the 'anemomind' repository so that you can download the latest changes.
1. Launch the virtual machine 'Debian 8 32bits.vbox' in VirtualBox
2. On the VM, do ```cd ~/anemomind/src/device/anemobox/anemonode```.
3. Put the repository in the state you want.
4. Make sure that you have a unique version tag in ```src/device/anemobox/anemonode/version.js```
5. Edit ```install.sh``` to either compile or not compile the C++ source code. Also make sure that ```HOST=192.168.2.1```. That should always be the IP address of the anemobox.
6. Connect your computer on which the VirtualBox is running to the local network of the anemobox.
7. Call ```bash install.sh```
8. Now you can connect to your anemobox and you will see that, for instance, the ```/anemonode``` subdirectory has been updated.
