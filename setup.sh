#!/usr/bin/env bash

# Load up .env
export $(egrep -v '^#' k8s/gke/.env | xargs)
set -e 

TAG=${1:-latest}

export REGISTRY=anemomind/
#REGISTRY=gcr.io/${PROJECT}/
cpp_image=${REGISTRY}anemomind_anemocppserver:${TAG}
web_image=${REGISTRY}anemomind_anemowebapp:${TAG}

# builidng images
echo "Creating images for tag: ${TAG}"

./src/build_docker_prod.sh "${PROJECT}" "${TAG}"

# pushing anemoweb image to gcp
# To use following command gcloud must be installed on the machine.
# if you are working more than one GCP projects the
# use gcloud init to select anemomind project.
#gcloud docker --authorize-only
for i in ${web_image} ${cpp_image}; do
    echo "Pushing $i..."
    docker push ${i}
done
