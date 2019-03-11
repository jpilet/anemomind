#include <server/common/ArgMap.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/logimport/LogLoader.h>

#include <iostream>

using namespace std;
using namespace sail;

static void summary(const NavDataset& dataset) {
  cout << "From " << dataset.lowerBound().toString()
    << " to " << dataset.upperBound().toString() << endl;

  dataset.outputSummary(&cout);
}

int main(int argc, const char **argv) {
  ArgMap cmdLine;

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  Array<ArgMap::Arg*> files = cmdLine.freeArgs();

  LogLoader loader;

  for (auto file : files) {
    string filename = file->value();
    if (loader.load(filename)) {
      cout << filename << ": loaded." << endl;
    } else {
      cerr << filename << ": failed to load." << endl;
    }
  }

  NavDataset dataset(loader.makeNavDataset());

  summary(dataset);

  return 0;
}
