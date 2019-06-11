#!/usr/bin/env bash
export GOOGLE_APPLICATION_CREDENTIALS="/home/wkhan/Downloads/anemomind-9b757e3fbacb.json"
mvn compile exec:java -Dexec.mainClass=com.anemomind.ProcessNewLogs \
     -Dexec.args="--sourcePath=/home/wkhan/bin/ \
                  --workerPath=/home/wkhan/worker/ \
                  --soBucket=localso
                  --concurrency=1" \
     -Pdirect-runner