/*
 * exportDispatcher.cpp
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#include <server/nautical/logimport/LogLoader.h>
#include <device/anemobox/DispatcherJson.h>

using namespace sail;

int main(int argc, const char **argv) {
  LogLoader loader;
  for (int i = 1; i < argc; i++) {
    loader.load(argv[i]);
  }
  json::outputJson(
      loader.makeDispatcher().get(),
      &(std::cout));
  return 0;
}


