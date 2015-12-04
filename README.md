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
```
Then build the test database. Make sure mongo is runnning, then run:
```
build_release/src/server/nautical/tiles/generateDevDB.sh
```
Now prepare the web server:
```
cd www2
npm install
bower install
```
and run it:
```
grunt server:dev
```

## Reference platform
The system compiles **at least** under Ubuntu 64-bit and Mac OSX 64-bit.

## Required dependencies:
  * Eigen 3
  * C++ compiler, such as GCC or LLVM/Clang
  * CMake build system.
  * Boost libraries: libboost-iostreams-dev, libboost-filesystem-dev, and probably other boost libraries.
  * The following packages, used by POCO:
    libssl-dev, ~~unixodbc-dev, libmysqlclient-dev,~~ libkrb5-dev
  * The following pacakges, used by Ceres: libeigen3-dev libsuitesparse-dev libcsparse2.2.3 libcxsparse2.2.3
  * Used by Armadillo: liblapack-dev, libblas-dev, libatlas3-base. See this page for help setting it up:
    http://danielnouri.org/notes/2012/12/19/libblas-and-liblapack-issues-and-speed,-with-scipy-and-ubuntu/
  * Armadillo
  * gnuplot (only necessary if you want to plot)

## Dependencies that are fetched automatically:
  * gtest
  * ADOL-C
  * POCO
  * ceres

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

## Platform specific notes

### Platforms using GCC version 5.x (e.g. Ubuntu 15.10)
Recent releases of GCC use a new ABI by default, in order
to support C++11 features. Read about it here:

http://developerblog.redhat.com/2015/02/05/gcc5-and-the-c11-abi/

This may cause linking errors, when mixing code compiled using GCC 4.x and GCC 5.y.
The sane way to deal with this is to either compile libraries from source using GCC 5
or only use official packages in the package manager. With Ubuntu 15.10, the
```libmongo-dev``` package is removed, so you will have to compile it. Getting the
outdated prebuilt deb-package is not a good idea, because it is likely compiled with
an outdated version of GCC. Instead, it is better to do this:
```
git clone https://github.com/mongodb/mongo-cxx-driver.git
cd mongo-cxx-driver
git checkout legacy
scons -j 20 --cache LINKFLAGS=-fuse-ld=gold CCFLAGS="-Wno-unused-variable -Wno-maybe-uninitialized"
sudo scons -j 20 --cache LINKFLAGS=-fuse-ld=gold CCFLAGS="-Wno-unused-variable -Wno-maybe-uninitialized" --prefix="/usr" install
```

### Mac OSX
TODO