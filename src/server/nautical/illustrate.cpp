/*
 * illustrate.cpp
 *
 *  Created on: 13 Oct 2016
 *      Author: jonas
 */

#include <server/common/ArgMap.h>
#include <server/common/TimeStamp.h>

using namespace sail;

struct Setup {
  std::string path;
  std::string from, to;
};

void makeIllustrations(const Setup &setup) {

}

int main(int argc, const char **argv) {

  Setup setup;
  ArgMap amap;
  amap.registerOption("--path", "Path to dataset")
      .store(&(setup.path))
      .required();
  amap.registerOption("--from", "YYYY-MM-DD").store(&(setup.to));
  amap.registerOption("--to", "YYYY-MM-DD").store(&(setup.from));

  auto status = amap.parse(argc, argv);
  switch (status) {
  case ArgMap::Done:
    return 0;
  case ArgMap::Continue:
    makeIllustrations(setup);
    return 0;
  case ArgMap::Error:
    return -1;
  };
  return 0;
}
