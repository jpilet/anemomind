/*
 *
 * NavCollection filterNavs(NavCollection navs) {
  GpsFilter::Settings settings;

  ArrayBuilder<Nav> withoutNulls;
  withoutNulls.addIf(navs, [=](const Nav &nav) {
    auto pos = nav.geographicPosition();
    return abs(pos.lat().degrees()) > 0.01
      && abs(pos.lon().degrees()) > 0.01;
  });
  auto results = GpsFilter::filter(withoutNulls.get(), settings);
  return results.filteredNavs().slice(results.inlierMask());
}
 */


#include <server/common/ArgMap.h>
#include <server/nautical/tiles/TileUtils.h>
#include <server/nautical/tiles/NavTileUploader.h>

using namespace sail;
int main(int argc, const char** argv) {
  ArgMap args;
  TileGeneratorParameters params;

  args.setHelpInfo("Generate vector tiles showing a boat trajectory from NMEA logs.");
  args.registerOption("--host", "MongoDB hostname").store(&params.dbHost);
  args.registerOption("--scale", "max scale level").store(&params.maxScale);
  args.registerOption("--maxpoints",
                      "downsample curves if they have more than <maxpoints> points")
    .store(&params.maxNumNavsPerSubCurve);

  std::string boatId;
  args.registerOption("--id", "boat id").setRequired().store(&boatId);

  std::string navPath;
  args.registerOption("--navpath", "Path to a folder containing NMEA files.")
    .setRequired().store(&navPath);

  std::string boatDat;
  args.registerOption("--boatDat", "Path to boat.dat").store(&boatDat);

  args.registerOption("--db", "Name of the db, such as 'anemomind' or 'anemomind-dev'")
      .setArgCount(1)
      .store(&params.dbName);

  args.registerOption("-u", "username for db connection")
      .setArgCount(1)
      .store(&params.user);

  args.registerOption("-p", "password for db connection")
      .setArgCount(1)
      .store(&params.passwd);

  std::string polarDat;
  args.registerOption("--polarDat", "Path to polar.dat").store(&polarDat);

  args.registerOption("--clean", "Clean all tiles for this boat before starting");

  if (args.parse(argc, argv) == ArgMap::Error) {
    return 1;
  }

  params.fullClean = args.optionProvided("--clean");

  processTiles(params, boatId, navPath, boatDat, polarDat);
  return 0;
}
