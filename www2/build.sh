#!/bin/bash

set -e

if [[ "x$VHOST" == "x" ]] ; then
  echo "VHOST is not defined, using 'client' as default."
  export VHOST="client"
fi

if [[ "$1" != "-n" ]] ; then
  # npm install is not really needed, because grunt build:dist will create a
  # dist/package.json file that contains the list of packages required for
  # production.

  bower install --allow-root --config.directory=$VHOST/bower_components
fi

grunt build:dist

rm -fR dist/node_modules/mail || true
sed 's#file:../nodemodules#file:../../nodemodules#' < package.json > dist/package.json

# npm install in dist folder
NODE_ENV=production npm install --production --prefix=dist --unsafe-perm=true --allow-root

# replace symlink with hard copy
# 
# Symlinks are found with:
# for i in $(find .. -type l) ; do [ -d $i ] && echo $i ; done
# ../www2/dist/node_modules/endpoint
# ../nodemodules/endpoint/node_modules/mangler
if [ -L "dist/node_modules/endpoint" ]; then
  rm -f "dist/node_modules/endpoint"
  cp -r "../nodemodules/endpoint" dist/node_modules
fi
if [ -L "dist/node_modules/endpoint/node_modules/mangler" ]; then
  rm -f "dist/node_modules/endpoint/node_modules/mangler"
  cp -r "../nodemodules/mangler" dist/node_modules
fi
