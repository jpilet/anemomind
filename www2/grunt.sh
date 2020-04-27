#!/bin/bash
set -e

# Cache npm install
checksum=$(cat ../nodemodules/mangler/package.json \
    ../nodemodules/endpoint/package.json \
    bower.json package.json | md5sum)

if [ "${checksum}" != "$(cat node_modules.checksum)" ]; then
    for i in mangler endpoint  ; do
      echo "Running npm install in ../nodemodules/${i}"
      npm -C "../nodemodules/${i}" install
    done

    # During the 1st run, npm will symlink endpoint.
    # During subsequent runs, if the symlink is present, it causes an error.
    # so we simply remove the symlink and let npm continue happily.
    rm -fR node_modules/endpoint || true
    npm install

    bower install --allow-root --config.directory=client/bower_components
    bower install --allow-root --config.directory=esalab/bower_components

    echo "${checksum}" > node_modules.checksum
fi

mkdir -p uploads/photos
mkdir -p /tmp/anemobackup

grunt $*
