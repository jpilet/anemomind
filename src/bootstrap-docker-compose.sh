#!/bin/bash
set -e

SRC_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." >/dev/null 2>&1 && pwd )"

export TAG=${TAG:-latest}

docker-compose -f "${SRC_ROOT}/docker-compose.yml" up -d 

# wait for mongo to come up
sleep 3

# create users
docker exec -i \
    -e "MONGO_URL=anemomind" \
    $(docker ps --filter "name=anemomongo" -q) \
    sh < "${SRC_ROOT}/src/create_dev_users.sh"

