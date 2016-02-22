#!/bin/bash
set -e

HOST=192.168.1.109

rm -fR node_modules/mail node_modules/endpoint || true
export NODE_ENV=production
npm install

node-gyp configure
node-gyp build

npm prune --production

../canutils/compile-can-utils.sh
../canutils/compile-iproute2.sh

EXCLUDE='--exclude="src" --exclude="*\.o" --exclude="binding.gyp" --exclude=README.md --exclude=install.sh --exclude=build/Release/obj'

git rev-parse HEAD > commit

ssh root@${HOST} rm -fR "/anemonode/*"
rsync -ar ${EXCLUDE} . root@${HOST}:/anemonode

#[ -e /anemonode ] || git clone git+ssh://anemobox@vtiger.anemomind.com/home/anemobox/anemobox.git /anemonode
#rm -fR /anemonode/*
#rsync -ar --exclude="src/*" --exclude="*.o" --exclude=id_rsa --exclude=install.sh . /anemonode


echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

