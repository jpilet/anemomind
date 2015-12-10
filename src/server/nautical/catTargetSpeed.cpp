/* Julien Pilet, 12.2015
 * Tool to convert a speed table contained in boat.dat in a json file.
 */
#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>
#include <server/common/ArgMap.h>

#include <iostream>
#include <stdio.h>

using namespace sail;
using namespace std;

int main(int argc, const char **argv) {
  ArgMap args;
  args.parse(argc, argv);

  if (args.freeArgs().size() == 0) {
    cerr << "usage: " << argv[0] << " <boat.dat>\n";
    return 1;
  }

  for (auto arg : args.freeArgs()) {
    TargetSpeedTable table;
    if (loadTargetSpeedTable(arg->value().c_str(), &table)) {
      printTargetSpeedAsJson(table);
    } else {
      cerr << arg->value() << ": can't load speed table.\n";
      return 1;
    }
  }
  return 0;
}
