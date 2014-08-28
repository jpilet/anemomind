#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>

#include <iostream>

void usage(const char *prog) {
  std::cerr << "Usage: " << prog << " <boat.dat>\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  TargetSpeedTable table;
  if (!loadTargetSpeedTable(argv[1], &table)) {
    std::cerr << argv[1] << ": can't load speed table\n";
    return 2;
  }

  plotTargetSpeedTable(table);

  return 0;
}

