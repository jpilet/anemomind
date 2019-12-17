#!/usr/bin/env bash

# Load up .env
export $(egrep -v '^#' k8s/gke/.env | xargs)
set -e 

cpp_image=gcr.io/${PROJECT_NAME}/anemomind_anemocppserver:latest
web_image=gcr.io/$PROJECT_NAME/anemomind_anemowebapp:latest

# builidng images
echo "Creating images....!"
docker build -t ${cpp_image} -f src/server/Dockerfile .
docker build -t ${web_image} -f www2/Dockerfile --build-arg CPP_DOCKER_IMAGE=${cpp_image} . 
echo "images created successfully...!" 

# pushing anemoweb image to gcp
# To use following command gcloud must be installed on the machine.
# if you are working more than one GCP projects the
# use gcloud init to select anemomind project.
gcloud docker --authorize-only && \
docker push ${web_image} && echo "Image pushed to GCP...!" || exit 1
