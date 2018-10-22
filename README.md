# main anemomind repository
This repository holds the code for 
  * The anemobox
  * The web interface
  * The computational backend

In ```anemomind-ios```, you will find the code for iOS devices.

# Quick dev setup guide
To build the cpp code in release mode:
```
mkdir build_release
cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4
```
*If you get a compilation error*, you can try to call ```make -j 4``` again until everything builds with no errors.

Then build the test database. Make sure mongo is runnning, then run:
```
build_release/src/server/nautical/tiles/generateDevDB.sh
```
Now prepare the web server:
```
mkdir www/db
cd ../www2
mkdir uploads
npm install
bower install
```
and run it:

```
grunt serve:dev
```

## Reference platform
The system compiles **at least** under Ubuntu 64-bit and Mac OSX 64-bit.

## Required dependencies:
### For C++
  * Eigen 3
  * C++ compiler, such as GCC or LLVM/Clang
  * CMake build system.
  * Boost libraries: libboost-iostreams-dev, libboost-filesystem-dev, libboost-system-dev, libboost-regex-dev,
    libboost-thread-dev, libboost-dev
  * The following packages, used by POCO:
    libssl-dev, ~~unixodbc-dev, libmysqlclient-dev,~~ libkrb5-dev
  * The following packages, used by Ceres: libeigen3-dev libsuitesparse-dev libcsparse2.2.3 libcxsparse2.2.3
  * Used by Armadillo: liblapack-dev, libblas-dev, libatlas3-base. See this page for help setting it up:
    http://danielnouri.org/notes/2012/12/19/libblas-and-liblapack-issues-and-speed,-with-scipy-and-ubuntu/
  * Armadillo
  * gnuplot (only necessary if you want to plot)
  * libprotobuf-dev
  * protobuf-compiler
  * cairo


Summary:

On debian/ubuntu:

    sudo apt-get install cmake libboost-iostreams-dev libboost-filesystem-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-dev libeigen3-dev libsuitesparse-dev libcxsparse3 liblapack-dev libblas-dev libatlas3-base libprotobuf-dev  protobuf-compiler libssl-dev libcairo2-dev build-essential git libarmadillo-dev f2c parallel mongo-clients catdoc clang libicu-dev libpython2.7

additionally, swift has to be installed: see https://www.cansurmeli.com/posts/install-swift-on-debian/

On macOS with macports:

    sudo port install cairo cmake eigen3 f2c armadillo protobuf-cpp catdoc

### For the web server
  * **node** and **npm**. Find packages here: ```https://nodejs.org/en/download/package-manager/```
  * **mocha**, for running unit tests: ```npm install -g mocha``` (possibly with ```sudo```)
  * **bower**, install with ```npm install -g bower``` (possibly with ```sudo```)
  * **grunt**, install with ```npm install -g grunt```, or should it be ```grunt-cli```? Try out yourself. (possibly with ```sudo```)

## Dependencies that are fetched automatically:
  * gtest
  * ADOL-C
  * POCO
  * ceres
  * mongodb c++ client

## Summary of steps to get started
The following steps cover building and testing all the code:
  1. Install dependencies
  2. Clone this repository
  3. From the root directory,
     ```mkdir build```
  4. ```cd build```
  5. ```cmake ../```
  6. ```make -j N```
     where ```N``` is the number of cores, e.g. 8
  7. ```make test```
  8. ```cd ../www2```
  9. ```npm install```
  10. ```bower install```
  11. ```grunt test```
  12. ```mocha```
  13. ```cd ../nodemodules/endpoint```
  14. ```npm install```
  15. ```mocha```
  16. ```cd ../mangler```
  17. ```npm install```
  18. ```mocha```
  19. ```cd ../../src/device/anemobox/anemonode/```
  20. ```npm install```
  21. ```mocha```

## Additional tests
Some things are difficult to test with unit tests. The pipeline that 
processes logs and upload tiles can be run, by first ensuring a that
a clean mongo server is running either
```
sudo killall mongod
mkdir /tmp/anemotestdb
mongod --dbpath /tmp/anemotestdb
```
or by doing from the project root
```
mkdir www/db
cd www2
grunt serve:dev
```

Then perform a build of the C++ code in your build directory (e.g. ```build```),
and run 
```
build/src/server/nautical/tiles$ sh generateDevDB.sh
```
Although this will not perform any correctness checks in particular, a great deal of the pipeline will nevertheless be run and it can therefore be a conventient tool when searching for bugs.

If you used the second example for starting mongodb indirectly using grunt ```grunt serve:dev```, the result will be visible on http://localhost:9000.


# Running in interactive shell ROOT

Install CERN's ROOT. On mac, do it with: ```sudo port install root6```
Then got to the build folder and type:
```make root```

To check that it worked, you can type:
```NavDataset().outputSummary(&std::cout)```

## Platform specific notes

### Mac OSX

TODO

```
brew install suite-sparse
```
