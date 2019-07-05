#!/usr/bin/env bash

########################################################################
# This scripts will create docker container(s) either in production
# envrironment or in devlopement environment.
# 
# In production following containers will be created:
#  anemomind_anemowebapp_1
#  anemomind_anemocppserver_1
#  anemomongo
#
# In developement anemomind_anemocppserver_1 container will be created. 
#
# input: environment [prod/dev]
# output: container(s) in eigther production or devlopement. 
########################################################################

if [ "$#" -ne 1 ]; then
    echo "USAGE: ./run.sh <environment>"
    echo "EXAPLE: ./run.sh prod"
    exit
fi

ENVIRONMENT=$1

if [ ${ENVIRONMENT} == "prod" ]; then
  
  echo "Execution started for production environment"
  # creating all containers using docker compose
  docker-compose up

elif [ ${ENVIRONMENT=} == "dev" ]; then
  
  echo "creating cpp server container for devlopment environment"
  
  # building docker image for cppserver as anemomind_anemocppserver
  # creating named volume for container
  # creating container as anemomind_anemocppserver_1 with bind mount and named volume
  
  docker build --target cppbuilder -t anemomind_anemocppserver -f src/server/Dockerfile . && \  
  docker volume create myvol && \
  docker run -d -v "$(pwd)"/src:/anemomind/src -v myvol:/anemomind/build -p 7188:22 --name anemomind_anemocppserver_1 -it anemomind_anemocppserver


else
  
  echo "Invalid option for run.sh please provide either 'prod' or 'dev'"
  exit

fi
