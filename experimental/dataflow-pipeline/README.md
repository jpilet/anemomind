# dataflow-pipeline

dataflow-pipeline runs cpp binaries on GCP with managed dataflow service.  
This pipeline executes three transformations: 
1. Reading message from pubsub topic:
   - when boat message published to pubsub, this transformation read the messge and send to the processboatlogs transformation.  
2. Processing Boat Logs:
   - Receives message from pubsub, checks the respective boat directory
   in google cloud storage, if found downloads all file of the directory in
   worker node.
   - After downloading files of the boat, it executes the ```nautical_processBoatLogs``` and updates the respective mongodb collections, 
   it also generates the boat.dat file.
   
3. Uploading VMG tables:    
   - It receives boat.dat generate in processboatlogs transformation, executes the ```nautical_catTargetSpeed``` binary,
   and updates the respective mongodb collections.
   
   
## Pre-requisites  

1. C++ binaries should be uploaded on Google Cloud Storage with location gs://<bucket_name>/<directory_name>/. e.g. ```gs://boat_binaries/bins/```  

2. Shared Object(SO) files should be uploaded on Google Cloud Storage with location gs://<bucket_name>/<directory_name>. e.g. ```gs://boat_binaries/libs/```   
NOTE: This will not require when static compilation of C++ code completed.  
3. boat files should be uploaded on the bucket with respective boat directories and same message should be sent to pubsub. This message should be in following format
  boat<boatid> e.g. ```boat5cda0f9f2153600028bfa397```  


## Usage
1. dataflow-pipeline can be run on GCP with DataFlow-Runner as well as in Local environment with Direct-Runner.  
2. To run dataflow-pipeline with Dataflow runner or with direct runner, user must have google credentials.
   Set ```GOOGLE_APPLICATION_CREDENTIALS``` environment variable if required.
3. To run dataflow-pipeline with Dataflow runner or with direct runner, user should use mvn compile exec command followed by different configuration parameters as -Dexec.args.
  
Followings are the list of all available configuration parameters for dataflow-pipeline: 

1.SourcePath: 
  - It is the path of google storage where binaries are stored.  
  - This is the required configuration parameter.  
  - usage ```--sourcePath=<path_to_cpp_binaries>```  
    
2.Concurrency:  
  - It is a concurrency level for binary execution and it must be > 0.  
  - This is also a required configuration parameter.  
  - usage ```--concurrency=3``` 

3.WorkerPath: 
  - It is a local path of worker node for I/O operations.  
  - It is an optional parameter with default value="/tmp/grid_working_files".  
  - usage ```--workerPath=<local_path_for_io>```  
  
4.WaitTime: 
  - The maximum time (in seconds) to wait for the sub-process to complete.
  - It is optional with default value=3600.
  - usage ```--waitTime=<timeInSeconds>``` 
    
5.OnlyUpLoadLogsOnError: 
  - uploads logs file to SourcePath from WorkerPath if error occurred.
  - It is optional with default value=true. 
  - usage ```--onlyUpLoadLogsOnError=<true/false>```

6.MongoDbUri:
  - Mongodb URI for anemomind
  - optional with default value=mongodb://localhost:27017/anemomind-dev
  - usage ```--mongoHost=<mongodb_uri>```


9.Topic:
  - Pubsub topic name where all boat messages published
  - optional with default value=projects/anemomind/topics/anemomind_log_topic
  - usage ```--topic=projects/<projectname>/topics/<topicName>```

10.BucketName:
   - Bucket name that contains all boat directories.
   - optional with default value=boat_logs
   - usage ```--bucketName=<bucketname>```

11.SoBucket:
   - Bucket name that contains directory for all Shared object files of c++ binaries.
   - optional with default value=boat_binaries.
   - This is a temporary parameter, will be removed once static binaries are used.
   - usage ```--soBucket=<bucketname>```  
   
### 1. Running On GCP with Dataflow Runner 
 
```processBoatLogs.sh``` file uses mvn compile exec with some configuration parameters. User can add/update the parameters in the file (optional)
and then run ```processBoatLogs.sh```.   

Note: For now publish pubsub message as 
```
gcloud beta pubsub topics publish <topicName> --message "<boatMessage>"
e.g. gcloud beta pubsub topics publish anemomind_log_topic --message "boat5cda0f9f2153600028bfa397"
``` 
after running the processBoatLogs.sh file. User can then checks the logs on 
GCP console in the stackdriver, and also cross verify the processed data on mongodb database.

### 2. Running On Local Machine with Direct Runner 
 
```processLocal.sh``` file uses mvn compile exec with some configuration parameters. User can add/update the parameters in the file as per their machine 
and then run ```processBoatLogs.sh```.   

Note:
1. Before running  ```processLocal.sh```, 
   - create bucket on GCP and then create directory ```libs``` in the bucket.
   - upload all locally generated shared objects of compiled c++ binaries into ```libs``` directory.
   - change value of ```--soBucket``` to this bucket in ```processLocal.sh``` file.
   - use locally compiled binary, and update value of ```--sourcePath``` to the local path where these binary resides in
   ```processLocal.sh``` file. 

2. publish pubsub message as 
```
gcloud beta pubsub topics publish <topicName> --message "<boatMessage>"
e.g. gcloud beta pubsub topics publish anemomind_log_topic --message "boat5cda0f9f2153600028bfa397"
``` 
after running the processLocal.sh file, User can then checks the logs on 
console and also cross verify the processed data on mongodb database.

3.Using the Direct Runner for testing and development helps ensure that pipelines are robust across different Beam runners. 
In addition, debugging failed runs can be a non-trivial task when a pipeline executes on a remote cluster. 
Instead, it is often faster and simpler to perform local unit testing on your pipeline code.
 