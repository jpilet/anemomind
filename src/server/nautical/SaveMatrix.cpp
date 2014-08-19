/*
 *  Created on: 2014-08-18
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/ArrayBuilder.h>
#include <iostream>
#include <fstream>


namespace {
  using namespace sail;

  const char *getOutname(int argc, const char **argv) {
    std::string prefix = "--out";
    for (int i = 1; i < argc-1; i++) {
      if (argv[i] == prefix) {
        return argv[i+1];
      }
    }
    return "outnavs.txt";
  }

  class ValueExtractor {
   public:
    ValueExtractor() {}
    ValueExtractor(const char *label, std::function<double(const Nav&)> e) :
      _label(label), _extractor(e) {}

    double extract(const Nav &n) {
      return _extractor(n);
    }

    std::string label() const {
      return _label;
    }
   private:
    std::string _label;
    std::function<double(const Nav &)> _extractor;
  };

  #define MAKE_VE(expr) ValueExtractor(#expr, [=](const Nav &x) {return expr;})

  Array<ValueExtractor> makeExtractors() {
    ArrayBuilder<ValueExtractor> builder;

    // Output in SI units
    builder.add(MAKE_VE(x.time().toSecondsSince1970()));
    builder.add(MAKE_VE(x.awa().radians()));
    builder.add(MAKE_VE(x.aws().metersPerSecond()));
    builder.add(MAKE_VE(x.externalTwa().radians()));
    builder.add(MAKE_VE(x.externalTws().metersPerSecond()));
    builder.add(MAKE_VE(x.gpsSpeed().metersPerSecond()));
    builder.add(MAKE_VE(x.gpsBearing().radians()));
    builder.add(MAKE_VE(x.magHdg().radians()));
    builder.add(MAKE_VE(x.externalTwa().radians()));
    builder.add(MAKE_VE(x.externalTws().metersPerSecond()));
    builder.add(MAKE_VE(x.geographicPosition().lon().radians()));
    builder.add(MAKE_VE(x.geographicPosition().lat().radians()));
    builder.add(MAKE_VE(x.geographicPosition().alt().meters()));

    return builder.get();
  }

  MDArray2d extractValues(Array<ValueExtractor> extractors, Array<Nav> data) {
    int rows = data.size();
    int cols = extractors.size();
    MDArray2d dst(rows, cols);
    for (int i = 0; i < rows; i++) {
      const Nav &nav = data[i];
      for (int j = 0; j < cols; j++) {
        dst(i, j) = extractors[j].extract(nav);
      }
    }
    return dst;
  }

  void outputExplanatoryLabels(Array<ValueExtractor> extractors, std::ostream *outFile) {
    *outFile << "% This file is intended to be read with Matlab, using 'load' with the '-ascii' option." << std::endl;
    *outFile << "% The meaning of each column is" << std::endl;
    const int len = extractors.size();
    for (int i = 0; i < len; i++) {
      *outFile << "%  Column " << i+1 << " of " << len << ": " << extractors[i].label() << std::endl;
    }
  }

  void outputMatrixData(MDArray2d data, std::ostream *file) {
    // http://stackoverflow.com/questions/554063/how-do-i-print-a-double-value-with-full-precision-using-cout

    file->precision(17);
    for (int i = 0; i < data.rows(); i++) {
      for (int j = 0; j < data.cols(); j++) {
        *file << data(i, j) << " ";
      }
      *file << std::endl;
    }
  }

  void saveNavsToMatrix(Array<Nav> navs, std::string filename) {
    Array<ValueExtractor> extractors = makeExtractors();
    MDArray2d data = extractValues(extractors, navs);


    std::ofstream file(filename);
    if (file.good()) {
      outputExplanatoryLabels(extractors, &file);
      outputMatrixData(data, &file);
    } else {
      std::cout << "Failed to open output file " << filename << std::endl;
    }
  }

  void help() {
    std::cout << "Options: \n"
        "  --navpath [path]                   Provide a"
          "path for the NMEA files. Defaults to the datas"
          "ets/Irene directory of the project.\n"
        "  --slice [from-index] [to-index]    Select a s"
         "ubset of the loaded navs. Defaults to the entire set.\n"
        "  --out [filename]                   Provide"
          "a filename for the file to output. Defaults to outnavs.txt\n";
  }
}

int main(int argc, const char **argv) {
  std::cout << "Loading navs..." << std::endl;
  sail::Array<sail::Nav> data = sail::getTestdataNavs(argc, argv);
  if (data.empty()) {
    help();
  }
  std::string outname = getOutname(argc, argv);
  std::cout << "Save matrix of " << data.size() << " navs to " << outname << "..." << std::endl;
  saveNavsToMatrix(data, outname);
  std::cout << "Done." << std::endl;
  return 0;
}
