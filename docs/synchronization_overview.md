# Synchronization overview

This is a summary of what we synchronize and how that is synchronized. It also tries to outline things that we could test. The idea is that we should read this document when we refactor the synchronization, so that we remember how the system should work.

## Brief anemobox overview
This is not so much related to synchronization, but it is good to know that most of the code on the anemobox is in ```/anemonode``` subdirectory. There is a local git repository, and the ```demo``` branch of that repository has a ```run.sh``` script which will replay log files.

The anemobox can receive these types of data:
  * script requests
  * incoming file transfers (in particular *git bundles* and *calibration parameters*)

Currently added using ```addPacketHandler(...)```, see ```anemonode/components/LocalEndpoint.js```.

## Brief server overview
The server accepts these types of packets (see ```www2/server/api/mailrpc/packet-callbacks.js```):
  * Incoming log files
  * Incoming script responses

## Figuring out the destination endpoint from the anemobox
Is done from within ```anemonode/components/LocalEndpoint.js```, with a call to ```boxId.getAnemoId(...)```.

## Log files from anemobox to server
When the box is launched, the ```anemonode/main.js``` will post all log files that it can find the the ```/media/sdcard/logs``` directory. This is where log files are written by the dispatcher. Then, the dispatcher will keep on posting a log file whenever a log file is produced. Every time a log file is posted on the ```anemobox```, it is moved to the ```sentlogs/``` subdirectory.

Once the iOS device is synchronized with the box, the log files are transferred to the iOS device. Then, when the iOS device is synchronized with the server, the log files are transferred to the server. 

The code on the server that will receive the logfiles is located in ```www2/server/api/```. It will first save the log file, and for every log file try to push it to the processing server.

As the server receives a packet with a log file, the fact that this file has been received is propagated back towards the box using the lower-bound-mechanism. On the box, it seems like logfiles in the ```sentlogs/``` directory are *not* removed automatically. We might want to fix that.

The above procedure, or the better part of it, should be tested by ```T2```.

## Remote scripts from server to anemobox, and their response.
The web api for box execution is in ```www2/server/api/boxexec```. A command line utility is found in ```utilites/RunRemoteScript.js```. The result can be seen using ```utilities/ViewRemoteScript.js```.

Once a script has been posted, it will propagate to the box.

On the box, it will execute, and once executed post a response, and then notify the iOS device via bluetooth that there is more data to be synchronized.

The incoming script response is currently handled by code in ```www2/server/api/boxexec/packet-callbacks.js```.

This functionality should be tested by ```T3```.

## File transfer from server to anemobox
This mechanism is used to
  * Post ```boat.dat``` file for the anemobox, with calibration and target speed data.
  * Post git bundles, that are used to remotely update the box.

It should be covered by ```T1``` and ```T4```.

## Ideas for improvement
  * Support delivery of several packets in parallel
  * Avoid expensive recoding of binary data.
  * Maybe clean up log files from ```sentlogs/``` when we know they have been received.
  * Some general refactoring to have less code that is easier to understand.

## Tests to write
  * ```T1```: The basic synchronization test that we currently have. It transfers one file from the server to the box, and one file from the box to the server (treating it as a log file). It checks that the files were received at either end. **OK, this test has been written**
  * ```T2```: Verify log file transfer. *Briefly:* Clean the files to transfer and maybe reset the DB, launch the replay of some NMEA0183 data (see the ```demo``` branch on the anemobox). After a while (5 minutes at least or when it says ```posted log file```), synchronize just as with the previous sync test that we wrote.
  * ```T3```: Verify script execution. *Briefly:* Clean server, box and phone, post a script on the server for remote execution, propagate it to the box, let the script execute their and post back its response, propagate response to server, verify that the response is what it should be.
  * ```T4```: Verify calibration parameter transfer: *Briefly:* Clean up the state of the system, let the server post some file with calibration parameters, propagate it to the box, make sure that the box correctly receives it and puts the file where it should be.
