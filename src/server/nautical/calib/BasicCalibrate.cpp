#include <string>

#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/nautical/calib/Calibrator.h>

using namespace sail;

int main(int argc, char **argv) {
  if (argc < 2) {
    LOG(FATAL) << "usage: " << argv[0] << " <data folder>";
    return 1;
  }

  sail::Calibrator calibrator;
  calibrator.setVerbose();
  if (!calibrator.calibrate(Poco::Path(argv[1]), "00000000")) {
    return 1;
  }

  std::string matfile = "/tmp/calibration.mat";
  calibrator.saveResultsAsMat(matfile.c_str());

  LOG(INFO) << "Calibration data saved in " << matfile
    << ". Run:\n"
    << "octave " << Env::SOURCE_DIR << "/src/server/nautical/calibration_info.m\n"
    << "to read it.";

  return 0;
}
