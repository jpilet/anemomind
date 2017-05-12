#include <device/anemobox/DispatcherTrueWindEstimator.h>

#include <iostream>

using namespace sail;
using namespace std;

namespace {

bool boatDatInfo(std::string path) {

  Dispatcher d;
  DispatcherTrueWindEstimator twe(&d);

  if (twe.loadCalibration(path)) {
    cout << path << ":\n";
    cout << twe.info();
    return true;
  } else {
    cerr << "Failed to load " << path << endl;
    return false;
  }
}

}  // namespace

int main(int argc, char* argv[]) {

  if (argc == 1) {
    return boatDatInfo("boat.dat") ? 0 : 1;
  }

  for (int i = 1; i < argc; ++i) {
    if (!boatDatInfo(argv[i])) {
      return 1;
    }
  }
  return 0;
}

