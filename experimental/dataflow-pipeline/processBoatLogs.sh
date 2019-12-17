#!/usr/bin/env bash

# Load up .env
export $(egrep -v '^#' .env | xargs)
echo "Fetching mongo db's ip for dataflow."
>temp.txt
for i in mongod-0 mongod-1 mongod-2
 do
 a=`kubectl describe services $i | grep "10.0.*" | cut -d ":" -f2`
 if [ $i != "mongod-2" ]; then
 echo -n "$a:27017,">>temp.txt
 else
 echo -n "$a:27017">>temp.txt
 fi
 done
mongo_ips=`cat temp.txt | tr -d ' '`
rm -f temp.txt

export GOOGLE_APPLICATION_CREDENTIALS="../../www2/anemomind-9b757e3fbacb.json"
mvn compile exec:java -Dexec.mainClass=com.anemomind.ProcessNewLogs \
     -Dexec.args="--runner=DataflowRunner --project=${PROJECT_NAME} \
                  --subnetwork=https://www.googleapis.com/compute/v1/projects/anemomind/regions/europe-west1/subnetworks/anemovpc-sn-europe-west1b \
                  --gcpTempLocation=gs://boat_logs/tmp \
                  --sourcePath=gs://boat_binaries/bins/ \
                  --workerPath=/home/anemomind/ \
                  --region=${REGION} \
                  --subnetwork=https://www.googleapis.com/compute/v1/projects/anemomind/regions/${REGION}/subnetworks/${SUBNET} \
                  --mongoDbUri=mongodb://main_admin:${MONGO_PWD}@${mongo_ips}/anemomind-dev?replicaSet=MainRepSet&authSource=admin \
                  --concurrency=10" \
     -Pdataflow-runner
