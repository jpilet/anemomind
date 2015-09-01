#!/bin/bash
set -e

rm -fR node_modules/mail node_modules/endpoint || true
npm install
node-gyp configure
node-gyp build

if [ -f ~/.ssh/id_rsa ]; then
  echo "SSH Key already installed"
else
  echo "installing SSH Key"
  mkdir -p ~/.ssh
  cp id_rsa* ~/.ssh
  chmod 600 ~/.ssh/id_rsa
  cat known_hosts >> ~/.ssh/known_hosts
fi

[ -e /anemonode ] || git clone git+ssh://anemobox@vtiger.anemomind.com/home/anemobox/anemobox.git /anemonode

rsync -ar --exclude=id_rsa --exclude=install.sh . /anemonode

git rev-parse HEAD > /anemonode/commit

echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

