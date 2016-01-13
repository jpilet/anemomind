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
  string boatId;
  string tableName = "vmgspeed";
  args.registerOption("--id", "Generate mongo script to store the table to the given boat id").store(&boatId);
  args.registerOption("--table", "Mongo table");
  args.parse(argc, argv);

  if (args.freeArgs().size() != 1) {
    cerr << "usage: " << argv[0] << " <boat.dat>\n";
    return 1;
  }

  for (auto arg : args.freeArgs()) {
    TargetSpeedTable table;
    if (loadTargetSpeedTable(arg->value().c_str(), &table)) {
      if (!boatId.empty()) {
        cout << "db." << tableName << ".update({_id: ObjectId(\""
          << boatId << "\")},\n"
          << "{_id: ObjectId(\"" << boatId << "\"), table:\n";
      }

      printTargetSpeedAsJson(table);

      if (!boatId.empty()) {
        cout << "}, { upsert: true });\n";
      }

    } else {
      cerr << arg->value() << ": can't load speed table.\n";
      return 1;
    }
  }
  return 0;
}
