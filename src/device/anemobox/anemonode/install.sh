#!/bin/bash
set -e

rm -fR node_modules/mail || true

npm install

node-gyp configure
node-gyp build

rsync -ar . /anemonode

git rev-parse HEAD > /anemonode/commit
