echo 'Installing updates.... '
sudo apt-get update

echo 'Installing dependencies.... '
sudo apt-get install -y cmake libboost-iostreams-dev libboost-filesystem-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-dev libeigen3-dev libsuitesparse-dev libcxsparse3 liblapack-dev libblas-dev libatlas3-base libprotobuf-dev protobuf-compiler libssl-dev libcairo2-dev build-essential git libarmadillo-dev f2c parallel mongodb-clients catdoc clang libicu-dev libpython2.7 libsqlite3-dev
sudo apt-get install -y build-essential cmake libblkid-dev e2fslibs-dev libboost-all-dev libaudit-dev libeigen3-dev libcairo2-dev libblas-dev liblapack-dev libarmadillo-dev libceres-dev
sudo apt-get install -y mongodb-server

NODE_VERSION=v8.11.3-linux-x64

echo 'Downloading NodeJs.. '
sudo mkdir -p /opt
sudo curl https://nodejs.org/download/release/v8.11.3/node-${NODE_VERSION}.tar.xz -f -o - | sudo tar -C /opt -xJf -

NODEJS_HOME=/opt/node-${NODE_VERSION}/bin
PATH=${NODEJS_HOME}:$PATH
echo 'export PATH="'${NODEJS_HOME}':$PATH"' >> /home/vagrant/.profile
echo 'export NODEJS_HOME="'${NODEJS_HOME}'"' >> /home/vagrant/.profile
echo 'export NODEJS_HOME="'${NODEJS_HOME}'"' >> /home/vagrant/.bashrc
echo 'export PATH="'${NODEJS_HOME}':$PATH"' >> /home/vagrant/.bashrc

echo 'Installing mocha...'
npm install -g mocha
echo 'Installing bower...'
npm install -g bower
echo 'Installing grunt...'
npm install -g grunt
npm install -g grunt-cli
