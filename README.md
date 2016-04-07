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
cd www
mkdir db
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

### For the web server
  * **node** and **npm**. Find packages here: ```https://nodejs.org/en/download/package-manager/```
  * **mocha**, for running unit tests: ```npm install -g mocha``` (possibly with ```sudo```)
  * **bower**, install with ```npm install -g bower``` (possibly with ```sudo```)
  * **grunt**, install with ```npm install -g grunt``` (possibly with ```sudo```)

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
scons -j 20 --cache LINKFLAGS=-fuse-ld=gold --c++11=on CCFLAGS="-Wno-unused-variable -Wno-maybe-uninitialized"
sudo scons -j 20 --cache LINKFLAGS=-fuse-ld=gold --c++11=on CCFLAGS="-Wno-unused-variable -Wno-maybe-uninitialized" --prefix="/usr" install
```
More information here: https://github.com/mongodb/mongo-cxx-driver/wiki/Download-and-Compile-the-Legacy-Driver#scons-options-when-compiling-the-c-driver. Note in particular that even if our code builds with this driver, it may still crash (with a segfault at runtime), in particular if **the C++ standard** differes between the different compiled code, as explained under *Important note about C++11/C++14*.

### Mac OSX
TODO