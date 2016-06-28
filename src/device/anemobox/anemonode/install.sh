#!/bin/bash
set -e

# TODO: get the IP from the command line
HOST=192.168.2.1
DEST=root@${HOST}:/anemonode

#DEST=/mnt/anemonode

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

EXCLUDE='--exclude=*.log --exclude=.*.sw[po] --exclude=src --exclude=*\.o --exclude=*.o.d --exclude=binding.gyp --exclude=README.md --exclude=install.sh --exclude=build/Release/obj --exclude=obj.target --exclude=*.node.d'

git rev-parse HEAD > commit

ssh root@${HOST} rm -fR "/anemonode/*"
#rm -fR "${DEST}/anemonode/*"
rsync -ar ${EXCLUDE} . ${DEST}

echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

