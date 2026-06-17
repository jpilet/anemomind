#!/bin/bash
set -e

# Get the directory of the script (src/device/anemobox)
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Repo root
REPO_ROOT="$( cd "$DIR/../../.." >/dev/null 2>&1 && pwd )"
ANEMOBOX_ROOT="$(realpath "$REPO_ROOT/../anemobox")"

echo "Building docker image for anemobox cross-compilation (i386/debian:jessie)..."
docker build --platform linux/386 -t anemobox-builder "$DIR"

echo "Running node-gyp in the docker container..."
docker run --rm \
    --platform linux/386 \
    -u "$(id -u):$(id -g)" \
    -v "/etc/passwd:/etc/passwd:ro" \
    -v "/etc/group:/etc/group:ro" \
    -v "$ANEMOBOX_ROOT:/anemobox" \
    -v "$REPO_ROOT:/anemomind" \
    -w "/anemomind/src/device/anemobox/anemonode" \
    anemobox-builder \
    bash -c "rsync -ar /anemobox/node_modules . && npm install && /usr/local/lib/node_modules/npm/bin/node-gyp-bin/node-gyp configure && /usr/local/lib/node_modules/npm/bin/node-gyp-bin/node-gyp build"

