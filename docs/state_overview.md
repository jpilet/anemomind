# State overview

List of different pieces of state that we need to care about. 
Here we only list state that survives a restart of the device, not in memory state.

I used http://www.tablesgenerator.com/markdown_tables to generate these tables.

## Box states
| Label            | Description                                      | Location                    | Initialized when                             | Updated when                                                            |
|------------------|--------------------------------------------------|-----------------------------|----------------------------------------------|-------------------------------------------------------------------------|
| Boot counter     | Keeps track of how many times the box was booted | /home/anemobox/bootcount    | /anemonode/run.sh is called                  | /anemonode/run.sh is called                                             |
| Recent log files | Log files that have not been posted yet          | /media/sdcard/logs          | The first log file is recorded               | A log file is recorded                                                  |
| Sent log files   | Log files that have been posted                  | /media/sdcard/logs/sentlogs | The first log file is posted                 | A log file is posted. Currently, sent log files are *never cleaned up*. |
| Local endpoint   | Database of packets to be transferred            | /media/sdcard/mail2/*.db    | The endpoint is accessed for the first time. | Every time we use it.                                                   |

## iOS app states
See AnemomindApp.xcdatamodeld

| Label                    | Description                                 | Location   | Initialized when         | Updated when       |
|--------------------------|---------------------------------------------|------------|--------------------------|--------------------|
| ANMBoat                  | All boats that the app is used with         | Data model |                          |                    |
| ANMEvent                 | Notes and sail changes. *Related to boat*.  | Data model |                          |                    |
| ANMLowerBound, ANMPacket | Used for packet synchronization             | Data model | On first synchronization | When synchronizing |
| ANMSail                  | What sails there are. *Related to boat*.    | Data model |                          |                    |
| ANMUser                  | The users using the app. *Related to boat*. | Data model |                          |                    |

## Web server states
See ```www2/server/config/environment/``` for the location of the different states for different server configurations.

| Label          | Description                                        | Location                                     | Initialized when                                     | Updated when                                                                                                                                             |
|----------------|----------------------------------------------------|----------------------------------------------|------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| Mongo database | Users, sails, boats, etc. (TODO: break this down)  | See the ```mongo.uri``` entry in the config. | When using Mongoose, see the ```*.model.js``` files. |                                                                                                                                                          |
| Uploads        | Uploaded files.                                    | See the ```uploadDir``` entry in the config. | On first use.                                        | When uploading photos (to ```photos/``` subdirectory. When uploading log files (to ```anemologs/```) subdirectory. Probably used by updates somehow too. |
| Endpoints      | Endpoints for every boat, used when synchronizing. | See the ```endpointDir``` in the config.     | On first use.                                        | Whenever we post data for a box on the server, or someone connects using the ```mailrpc``` API.                                                          |

## Vtiger states
| Label     | Description                                     | Location                                 | Initialized when | Updated when                                 |
|-----------|-------------------------------------------------|------------------------------------------|------------------|----------------------------------------------|
| Log files | A directory for the log files of all the boats. | ```/home/anemomind/userlogs/anemologs``` | On first use.    | When new log files arrive on the web server. |
| Photos    | A directory for the photos of all the users.    | ```/home/anemomind/userlogs/photos```    | On first use.    | When new photos arrive.                      |
