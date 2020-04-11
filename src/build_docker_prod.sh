#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

"${DIR}/docker_run.sh" -i ../src/compile_and_cp_bin.sh

PROJECT=${2:-anemomind}
export REGISTRY=anemomind/
#REGISTRY=gcr.io/${PROJECT}/

TAG=${1:-latest}

# TODO: add -t option with the appropriate image name and tag.
docker build -f Dockerfile.prod -t ${REGISTRY}anemomind_anemocppserver:${TAG} "${DIR}/.."

docker build -t anemomind/web_dev:${TAG} -f www2/Dockerfile \
    --target grunt-build "${DIR}/.."

docker build -t ${REGISTRY}anemomind_anemowebapp:${TAG} -f www2/Dockerfile \
         --build-arg MONGO_URL=mongodb://anemomongo:27017/anemomind-dev \
         --build-arg GCLOUD_PROJECT=${PROJECT}\
         --build-arg GCS_KEYFILE=/anemomind/www2/anemomind-9b757e3fbacb.json\
         --build-arg GCS_BUCKET=boat_logs\
         --build-arg USE_GS="true" \
         --build-arg PUBSUB_TOPIC_NAME=anemomind_log_topic \
         --build-arg CPP_DOCKER_IMAGE=gcr.io/$PROJECT/anemomind_anemocppserver:$TAG \
         "${DIR}/.."

