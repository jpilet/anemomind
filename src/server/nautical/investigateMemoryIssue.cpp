/*
 * investigateMemoryIssue.cpp
 *
 *  Created on: 12 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <iostream>

using namespace sail;

int main() {
  LogLoader loader;
  loader.load("/Users/jonas/data/boat573b14bbf437a48c0ce3ad54");
  auto ds = loader.makeNavDataset();
  int counter = 0;

  std::cout << "Please note the current memory consumption." << std::endl;
  std::cout << "Do you want to access a GPS sample? 1 or 0" << std::endl;

  int answer;
  std::cin >> answer;

  ds.evaluateMerged();

  std::vector<NavDataset> shallowCopies;
  while (true) {
    auto copy = ds.dup();

    if (answer != 0) {
      std::cout << "Access the first sample in the copy: " <<
          copy.samples<GPS_POS>().first().time.toString();
    }

    shallowCopies.push_back(copy);
    std::cout << " Number of shallow copies so far: "
        << shallowCopies.size() << std::endl;
  }

  return 0;
}


