# Semi-automatic synchronization test procedure

## CONFIGURATION

There is this line in in ```www/synctest/get_devbox_boatid.sh```. It needs to be edited for the box that we are using:
```
mongo --quiet anemomind-dev --eval 'db.boats.findOne({anemobox: "784b87a0b162"})["_id"].valueOf()'
```
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
 - **anemobox**: A terminal window to the ```/anemonode/synctest``` subdirectory on the anemobox.
 - **anemomind-ios**: A terminal window at the anemomind-ios repository
 - **www2/synctest**: A terminal window pointing at the ```www2/synctest``` subdirectory of the anemomind repository.
 - **iOS**: The iOS device
 - **XCode**: An XCode window, configured to communicate with the iOS app on the device.
 
### Steps to perform

#### Common initial steps
There are currently three tests in place, indexed 1, 2, 3. Replace X by 1, 2 or 3, where appropriate in the description below.

 - Connect your iOS device to your Macbook.
 - Open all of the above interface.
 - **anemomind/www2**: Make sure that no server is running.
 - **XCode**, **iOS**: Make sure the app isn't running. If it is, press the stop button in XCode.

 - **anemobox**: Make sure that there is no anemonode process running.
 - **anemomind/www2**: Call ```./sync_runX.sh``` to put the web server in a clean state and start the web server.
 - **anemobox**: Call ```./sync_runX.sh``` to put the anemobox in a clean state and start the service.

#### Specific steps for test 1 and 2
 - **iOS**: **Connect the iOS device to the anemobox wifi network**.
 - **anemomind-ios**: Call ```./prepare_sync_test.sh``` to patch the source code so that the app will connect to the IP address of the Macbook and the database will be reset.
 - **XCode**: Press on the "Play" button to compile the project and run the app on the iOS device connected with the cable.
 - **iOS**: **Only for test 2:** Wait 5 minutes.
 - **iOS**: Open the *Home* view and synchronize with the box.
 - **iOS**: Connect to the local office network.
 - **iOS**: Go to the *Home* view and synchronize with the web server.
 - **iOS**: Connect to the anemobox local network again.
 - **iOS**: Go to the *Home* view and synchronize with the anemobox.

#### Specific steps for test 3
 - **iOS**: **Connect the iOS device to the local office network**.
 - **anemomind-ios**: Call ```./prepare_sync_test.sh``` to patch the source code so that the app will connect to the IP address of the Macbook and the database will be reset.
 - **XCode**: Press on the "Play" button to compile the project and run the app on the iOS device connected with the cable.
 - **iOS**: Open the *Home* view and synchronize with the web server.
 - **iOS**: Connect to the anemobox network.
 - **iOS**: Go to the *Home* view and synchronize with the anemobox.
 - **iOS**: Connect to the local office network.
 - **iOS**: Go to the *Home* view and synchronize with the web server.

#### Common final steps
 - **anemomind/www2**: Press ```Ctrl+C``` to halt the web server.
 - **anemobox**: Press ```Ctrl+C``` to halt the anemonode server. If the box restarts, just try to log in again once it has restarted.
 - **XCode**: Press the "Stop" button to close the app.

By now, the test should have succeeded. To check that, do the following:

 - **anemobox**: Call ```./sync_checkX.sh```. **You should see the message ** ```Passed :-)``` being printed out.
 - **anemomind/www2**: Call ```./sync_checkX.sh```. **NOTE:** For ```sync_check2.sh``` you will need to provide two arguments given by ```sync_check2.sh``` on the anemobox. **You should see the message **```Passed :-)``` being printed out.
