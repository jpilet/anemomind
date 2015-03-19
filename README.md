# anemomind optimization code
This repository holds the code to perform numerical calculations
on nautical recordings. The majority of the code is written in C++.

# Quick dev setup guide
To build the cpp code in release mode:

    mkdir build_release; cd build_release; cmake .. -DCMAKE_BUILD_TYPE=Release; make -j 4

Then build the test database. Make sure mongo is runnning, then run:

    build_release/src/server/nautical/tiles/generateDevDB.sh

Now run prepare the web server:

   cd www2
   npm install
   bower install

and run it:

   grunt server:dev


## Reference platform
The system compiles **at least** under Ubuntu 64-bit and Mac OSX 64-bit.

## Required dependencies:
  * C++ compiler, such as GCC or LLVM/Clang
  * CMake build system
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

## Summary of steps to get started:
  1. Install dependencies
  2. Clone this repository
  3. From the root directory,
     mkdir build
  4. cd build
  5. cmake ../
  6. make -j N
     where N is the number of cores, e.g. 8
  7. make test
