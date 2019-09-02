#!/usr/bin/env bash
export GOOGLE_APPLICATION_CREDENTIALS="../../www2/anemomind-9b757e3fbacb.json"
mvn compile exec:java -Dexec.mainClass=com.anemomind.ProcessNewLogs \
     -Dexec.args="--runner=DataflowRunner --project=anemomind --region=europe-west1 \
                  --subnetwork=https://www.googleapis.com/compute/v1/projects/anemomind/regions/europe-west1/subnetworks/anemovpc-sn-europe-west1b \
                  --gcpTempLocation=gs://boat_logs/tmp \
                  --sourcePath=gs://boat_binaries/bins/ \
                  --workerPath=/home/anemomind/ \
                  --mongoDbUri=mongodb://main_admin:anemo123@10.0.0.62:27017,10.0.0.61:27017,10.0.0.60:27017/anemomind-dev?replicaSet=MainRepSet&authSource=admin \
                  --concurrency=10" \
     -Pdataflow-runner 
