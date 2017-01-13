# Semi-automatic synchronization test procedure

## Prerequisites
 - A cable to connect the iPad to a computer
 - A Macbook with XCode, ```anemomind-ios``` repository and ```anemomind``` repository
 - The beige 3d-printed anemobox, configured with id ```box784b87a0b162``` and boat ```57f678e612063872e749d481```
 - An iPad with the Anemomind configured for the above box and boat

## Some info
 - The anemobox has a wifi network named ```anemobox```. The key is written on the box.
 - Login credentials to the anemobox are written on it.

## Test procedure
We will be referring to a number of interfaces between which you need to switch to carry out the test:
 - **anemobox**: A terminal window to the ```/anemonode``` subdirectory on the anemobox.
 - **anemomind-ios**: A terminal window at the anemomind-ios repository
 - **anemomind/www2**: A terminal window pointing at the ```www2/``` subdirectory of the anemomind repository.
 - **iOS**: The iOS device
 - **XCode**: An XCode window, configured to communicate with the iOS app on the device.
 
### Steps to perform
 - Connect your iOS device to your Macbook.
 - Open all of the above interface.
 - **anemomind/www2**: Make sure that no server is running.
 - **XCode**, **iOS**: Make sure the app isn't running. If it is, press the stop button in XCode.

 - **anemobox**: Make sure that there is no anemonode process running.
 - **anemomind/www2**: Call ```./sync_start_test.sh``` to put the web server in a clean state and start the web server. (You may have to call ```chmod u+x sync_start_test.sh``` first).
 - **anemobox**: Call ```./sync_start_test.sh``` to put the anemobox in a clean state and start the service.
 - **iOS**: Connect the iOS device to the ```anemobox``` wifi network from the sysstem settings.
 - **anemomind-ios**: Call ```./prepare_sync_test.sh``` to patch the source code so that the app will connect to the IP address of the Macbook and the database will be reset.
 - **XCode**: Press on the "Play" button to compile the project and run the app on the iOS device connected with the cable.
 - **iOS**: Open the *Home* view and synchronize with the box.
 - **iOS**: Connect to the same local network as the web server from the system settings.
 - **iOS**: Go to the *Home* view and synchronize with the web server
 - **iOS**: Connect to the ```anemobox``` local network again
 - **iOS**: Go to the *Home* view and synchronize with the anemobox.
 - **anemomind/www2**: Press ```Ctrl+C``` to halt the web server.
 - **anemobox**: Press ```Ctrl+C``` to halt the anemonode server. If the box restarts, just try to log in again once it has restarted.
 - **XCode**: Press the "Stop" button to close the app.

By now, a file should have been successfully transferred from the box to the server, and another file should have been transferred from the server to the box. Let's check that this is the case.

 - **anemobox**: Call ```./sync_check.sh```. **You should see the message ** ```Passed :-)``` being printed out.
 - **anemomind/www2**: Call ```./sync_check.sh```. **You should see the message **```Passed :-)``` being printed out.
