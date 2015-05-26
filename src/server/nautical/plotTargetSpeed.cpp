#include <device/Arduino/libraries/TargetSpeed/TargetSpeed.h>

#include <server/plot/extra.h>
#include <iostream>

using namespace sail;

void plotTargetSpeedTable(const TargetSpeedTable& table) {
  GnuplotExtra plot;
  plot.set_grid();
  plot.set_style("lines");
  plot.set_xlabel("Wind Speed (knots)");
  plot.set_ylabel("VMG (knots)");

  const int numEntries = TargetSpeedTable::NUM_ENTRIES;
  Arrayd X = Arrayd::fill(
      numEntries, [&](int i) { return double(table.binCenter(i)); });
  Arrayd upwind(numEntries);
  Arrayd downwind(numEntries);

  for (int i = 0; i < numEntries; ++i) {
    upwind[i] = table._upwind[i];
    downwind[i] = table._downwind[i];
    std::cout << static_cast<double>(table.binCenter(i))
      << ", " << upwind[i] << ", " << downwind[i] << "\n";
  }
   
  plot.plot_xy(X, upwind, "Upwind");
  plot.plot_xy(X, downwind, "Downwind");
  plot.show();
}

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

