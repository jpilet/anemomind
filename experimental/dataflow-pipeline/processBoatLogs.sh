#!/usr/bin/env bash
export GOOGLE_APPLICATION_CREDENTIALS="/home/wkhan/Downloads/anemomind-9b757e3fbacb.json"
mvn compile exec:java -Dexec.mainClass=com.anemomind.ProcessNewLogs \
     -Dexec.args="--runner=DataflowRunner --project=anemomind \
                  --gcpTempLocation=gs://boat_logs/tmp \
                  --sourcePath=gs://boat_binaries/bins/ \
                  --workerPath=/home/anemomind/ \
                  --mongoHost=35.239.4.38 \
                  --concurrency=10" \
     -Pdataflow-runner 
