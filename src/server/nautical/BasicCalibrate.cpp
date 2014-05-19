#include <server/nautical/Calibrator.h>
#include <server/common/logging.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    LOG(FATAL) << "usage: " << argv[0] << " <data folder>";
    return 1;
  }

  sail::Calibrator calibrator;
  return calibrator.calibrate(Poco::Path(argv[1]), "00000000")
    ? 0 : 1;
}
