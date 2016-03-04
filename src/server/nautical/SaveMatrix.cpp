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
  using namespace sail::NavCompat;

  std::string getOutname(ArgMap &amap) {
    if (amap.optionProvided("--out")) {
      return amap.optionArgs("--out")[0]->value();
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

  MDArray2d extractValues(Array<ValueExtractor> extractors, NavDataset data) {
    int rows = getNavSize(data);
    int cols = extractors.size();
    MDArray2d dst(rows, cols);
    for (int i = 0; i < rows; i++) {
      Nav nav = getNav(data, i);
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

  int saveNavsToMatrix(NavDataset navs, std::string filename) {
    Array<ValueExtractor> extractors = makeExtractors();
    MDArray2d data = extractValues(extractors, navs);


    std::ofstream file(filename);
    if (file.good()) {
      outputExplanatoryLabels(extractors, &file);
      outputMatrixData(data, &file);
      return 0;
    } else {
      std::cout << "Failed to open output file " << filename << std::endl;
      return -1;
    }
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  amap.setHelpInfo("A program to output the navs as a matrix in text format.");
  registerGetTestdataNavs(amap);
  amap.registerOption("--out", "A filename for the output. Defaults to outnavs.txt.").setArgCount(1);
  if (amap.parse(argc, argv) != ArgMap::Error) {
    std::cout << "Loading navs..." << std::endl;
    sail::NavDataset data = sail::getTestdataNavs(amap);
    if (isEmpty(data)) {
      amap.dispHelp(&std::cout);
      return -1;
    }
    std::string outname = getOutname(amap);
    std::cout << "Save matrix of " << getNavSize(data) << " navs to " << outname << "..." << std::endl;
    return saveNavsToMatrix(data, outname);
  }
  return -1;
}
