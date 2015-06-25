#!/bin/bash
set -e

#rm -fR node_modules/mail || true
#npm install
#node-gyp configure
#node-gyp build

[ -f ~/.ssh/id_rsa ] || (mkdir -p ~/.ssh && cp id_rsa ~/.ssh)

[ -e /anemonode ] || git clone git+ssh://anemobox@vtiger.anemomind.com/home/anemobox/anemobox.git /anemonode

rsync -ar --exclude=id_rsa --exclude=install.sh . /anemonode

git rev-parse HEAD > /anemonode/commit

echo "Installed. After testing, please validate the release files with: "
echo "  cd /anemonode ; git add . ; git commit ; git push"
echo "or invalidate with:"
echo "  cd /anemonode ; git reset --hard"

