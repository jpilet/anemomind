#!/bin/bash
set -e

# TODO: get the IP from the command line
# 2.1
HOST=192.168.2.1
DEST=root@${HOST}:/anemonode

#DEST=/mnt/anemonode

# avoid rebuilding, because rebuilding will make npm updates the dependencies
# in node_modules, which in turn will lead to a large patch
if false; then

  rm -fR node_modules/mail node_modules/endpoint || true
  export NODE_ENV=production
  npm install --production

  node-gyp configure
  node-gyp build

  npm prune --production
  npm dedupe

  ../canutils/compile-can-utils.sh
  ../canutils/compile-iproute2.sh
fi

strip build/Release/anemonode.node

EXCLUDE='--exclude=*.log --exclude=.*.sw[po] --include=src/*.js --exclude=src/*.* --include=src/*.js --exclude=*\.[oadh] --exclude=binding.gyp --exclude=README.md --exclude=install.sh --exclude=build/Release/obj --exclude=obj.target --exclude=*.target.mk --exclude=Makefile --exclude=*.tar.gz --exclude=*.gypi --exclude=build/Debug'

git rev-parse HEAD > commit

ssh root@${HOST} rm -fR "/anemonode/*"
rsync -ar ${EXCLUDE} . ${DEST}

echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

