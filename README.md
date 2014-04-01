# anemomind optimization code
This repository holds the code to perform numerical calculations
on nautical recordings. The majority of the code is written in C++.

## Reference platform
The system compiles **at least** under Ubuntu 64-bit and Mac OSX 64-bit.

## Required dependencies:
  * C++ compiler, such as GCC or LLVM/Clang
  * CMake build system
  * The following packages, used by POCO:
    libssl-dev, unixodbc-dev, libmysqlclient-dev, libkrb5-dev
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