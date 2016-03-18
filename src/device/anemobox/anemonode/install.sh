#!/bin/bash
set -e

# TODO: get the IP from the command line
HOST=192.168.1.106

# avoid rebuilding, because rebuilding will make npm updates the dependencies
# in node_modules, which in turn will lead to a large patch
if false; then

  rm -fR node_modules/mail node_modules/endpoint || true
  export NODE_ENV=production
  npm install

  node-gyp configure
  node-gyp build

  npm prune --production
  npm dedupe

  ../canutils/compile-can-utils.sh
  ../canutils/compile-iproute2.sh
fi

EXCLUDE='--exclude=*.log --exclude=.*.sw[po] --exclude=src --exclude=*\.o --exclude=*.o.d --exclude=binding.gyp --exclude=README.md --exclude=install.sh --exclude=build/Release/obj'

git rev-parse HEAD > commit

ssh root@${HOST} rm -fR "/anemonode/*"
rsync -ar ${EXCLUDE} . root@${HOST}:/anemonode

echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

