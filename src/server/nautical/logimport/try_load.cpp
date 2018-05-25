#include <server/common/ArgMap.h>
#include <server/nautical/NavDataset.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/common/string.h>

#include <iostream>

using namespace std;
using namespace sail;

static void summary(const NavDataset& dataset) {
  cout << "From " << dataset.lowerBound().toString()
    << " to " << dataset.upperBound().toString() << endl;

  dataset.outputSummary(&cout);
}


std::vector<std::string> listChannels(const NavDataset& ds) {
  std::vector<std::string> result;
#define DISP_CHANNEL(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  if (ds.sourcesForChannel(HANDLE).size() > 0) { result.push_back(SHORTNAME); } 
  FOREACH_CHANNEL(DISP_CHANNEL)
#undef DISP_CHANNEL
  return result;
}

bool containsData(const NavDataset& dataset) {
 return dataset.lowerBound().defined() && dataset.upperBound().defined();
}

int main(int argc, const char **argv) {
  ArgMap cmdLine;
  std::string path = "";

  cmdLine.registerOption("-j", "Output result in JSON format")
    .setArgCount(0);

  cmdLine.registerOption("-C", "<dir> Set the working directory")
    .store(&path);

  if (cmdLine.parse(argc, argv) != ArgMap::Continue) {
    return -1;
  }

  if (path.size() > 0) {
    path += "/";
  }

  Array<ArgMap::Arg*> files = cmdLine.freeArgs();

  std::vector<std::string> objects;

  for (auto file : files) {
    string filename = file->value();
    LogLoader loader;
    bool r = loader.loadFile(path + filename);
    NavDataset dataset = loader.makeNavDataset()
      .fitBounds();  // fitBounds defines lowerBound and upperBound.

    stringstream ss;
    if (containsData(dataset)) {
      ss << "{\n"
        << "  \"name\": \"" << filename << "\",\n"
        << "  \"start\": \"" << dataset.lowerBound().toString() << "\"\n"
        << "  \"duration_sec\": "
          << (dataset.upperBound() - dataset.lowerBound()).seconds() << ",\n"
        << "  \"data\": \"" << join(listChannels(dataset), ", ") << "\"\n"
        << "}\n";
    } else {
      ss << "{\n"
        << "  \"name\": \"" << filename << "\",\n"
        << "  \"error\": \"Failed to load.\"\n"
        << "}\n";
    }
    objects.push_back(ss.str());
  }
  cout << "[" << join(objects, ",") << "]\n";

  return 0;
}
