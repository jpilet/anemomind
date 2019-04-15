echo 'Installing updates.... '
sudo apt-get update

echo 'Installing dependencies.... '
sudo apt-get install -y cmake libboost-iostreams-dev libboost-filesystem-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-dev libeigen3-dev libsuitesparse-dev libcxsparse3 liblapack-dev libblas-dev libatlas3-base libprotobuf-dev protobuf-compiler libssl-dev libcairo2-dev build-essential git libarmadillo-dev f2c parallel mongodb-clients catdoc clang libicu-dev libpython2.7 libsqlite3-dev
sudo apt-get install -y build-essential cmake libblkid-dev e2fslibs-dev libboost-all-dev libaudit-dev libeigen3-dev libcairo2-dev libblas-dev liblapack-dev libarmadillo-dev libceres-dev
sudo apt-get install -y mongodb-server

echo 'Downloading NodeJs.. '
sudo curl -O https://nodejs.org/download/release/v8.11.3/node-v8.11.3-linux-x64.tar.xz
echo 'Extracting tar.. '
sudo tar -xf node-v8.11.3-linux-x64.tar.xz
echo 'Copying to opt folder '
sudo cp -r node-v8.11.3-linux-x64 /opt/

echo 'Setting path variables... '
echo 'export NODEJS_HOME=/opt/node-v8.11.3-linux-x64/bin' >> /home/vagrant/.profile
echo 'export PATH=$NODEJS_HOME:$PATH' >> /home/vagrant/.profile
echo 'export NODEJS_HOME=/opt/node-v8.11.3-linux-x64/bin' >> /home/vagrant/.bashrc
echo 'export PATH=$NODEJS_HOME:$PATH' >> /home/vagrant/.bashrc

echo 'export NODEJS_HOME=/opt/node-v8.11.3-linux-x64/bin' >> ~/.profile
echo 'export PATH=$NODEJS_HOME:$PATH' >> ~/.profile
echo 'export NODEJS_HOME=/opt/node-v8.11.3-linux-x64/bin' >> ~/.bashrc
echo 'export PATH=$NODEJS_HOME:$PATH' >> ~/.bashrc

echo 'Installing mocha...'
npm install -g mocha
echo 'Installing bower...'
npm install -g bower
echo 'Installing grunt...'
npm install -g grunt
npm install -g grunt-cli