# Repeatable sync test procedure

*Description*: A fully repeatable test procedure that starts from a well defined state.
*References*: ```sync_test_procedure.md```

This is a complementary, more rigorous, procedure for testing the synchronization. We want a test procedure that is *truly repeatable* and does not assume the system to be in a particular state.



## Steps
0. Call ```www2/synctest/refresh_boxid.sh```
1. Launch the web server.
2. On the laptop, go to ```cd www2/synctest``` and call ```NODE_ENV=development ./reset_web_server.sh```. That will reset the web server and add a test user, ```test@anemomind.com``` with password ```anemoTest```.
3. In the ```anemomind-ios``` subdirectory, call ```./prepare_full_reset.sh``` and ```./prepare_sync_test.sh```.
4. On the box, call ```./factory_reset.sh``` from the ```/anemonode``` directory. 
5. Launch the box service, by calling ```NO_WATCHDOG=1 ./synctest/sync_run1.sh``` or just ```NO_WATCHDOG=1 ./run.sh```. TODO: *In the current implementation, a sync won't be triggered when a boat id is assigned, but we have a task for that. Therefore, there is no point in trying to run the test from here. All we want is to make all devices being aware of each other.*.
6. Connect the app to the local network, ```anemomind2```
7. Compile and launch the app.
8. Log in on the app using the username ```test@anemomind.com```.
9. Change the local network of the tablet to ```Anemobox```.
10. Click on the image to sync with the box. At some point, you should see a message that you can "Create a new boat" for this box.
11. Create the boat.
12. Sync with the box. Note: The box might not yet know about the server.
13. Change local network of the tablet to ```anemomind2```.
14. Sync with the server.
15. Connect the tablet to the ```Anemobox``` network and sync with the box from within the app.
15. If you log in on the web page, you should see your new boat there.
16. Close the web server, the app (from withing XCode) and the box service.
17. In the ```anemomind-ios``` subdirectory, undo your previous changes by calling ```git stash```.
18. *Now all devices are aware of each other*. Continue with the tests, as explained in ```sync_test_procedure.md```.
