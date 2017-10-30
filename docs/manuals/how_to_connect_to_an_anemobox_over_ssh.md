# Steps

  0. Preparations: Put the following in ~/.ssh/config

```
host box
    HostName anemobox.local
    User root
    StrictHostKeyChecking no
    UserKnownHostsFile /dev/null
```
  1. Connect the yellow power cable to the right one of the two connectors on the anemobox.
  2. The wifi of the anemobox should appear among the local networks available. The name of the wifi network is also printed on the label of the anemobox (ssid). 
  3. Connect to the wifi of the anemobox using the password printed on the label on the box.
  4. From a terminal, connect to the anemobox over ssh: ```ssh root@anemobox.local``` and use the password ```sui91```.
  5. Once connected, call ```./disable_watchdog.sh```, followed by ```systemctl stop anemobox```.
  6. You can now do what you want with the box.
