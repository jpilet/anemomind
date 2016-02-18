/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/NKEParser.h>
#include <server/common/logging.h>
#include <iostream>
#include <server/common/PhysicalQuantityIO.h>
#include <server/common/correlation.h>

using namespace sail;

namespace {
  void readIJ(ArgMap &amap, const std::string key, int *i, int *j) {
    auto args = amap.optionArgs(key);
    args[0]->tryParseInt(i);
    args[1]->tryParseInt(j);
  }




  double ncc(NKEArray X, NKEArray Y) {
    if (X.unit()->isAngle() && Y.unit()->isAngle()) {
      return normalizedCrossCorrelation(X.angles(), Y.angles(), Angle<double>::degrees(1.0));
    } else if (X.unit()->isVelocity() && Y.unit()->isVelocity()) {
      return normalizedCrossCorrelation(X.velocities(), Y.velocities(), Velocity<double>::knots(1.0));
    } else {
      LOG(FATAL) << "Unknown/incompatable units when evaluating normalized cross correlation.";
      return NAN;
    }
  }
}


// /home/jonas/data/datasets/nke/ConvLorient-Barcelone/rapport/28_11_2014_15_00_00.csv
int main(int argc, const char **argv) {
  ArgMap amap;
  std::string filename;
  amap.registerOption("--file", "Provide name of CSV file to parse").store(&filename).required();
  amap.registerOption("--angle", "Provide row and col inds to view angle value").setArgCount(2);
  amap.registerOption("--velocity", "Provide row and col inds to view velocity value").setArgCount(2);
  amap.registerOption("--duration", "Provide row and col inds to view duration value").setArgCount(2);
  amap.registerOption("--ncc", "Compute the normalized cross correlation between two columns").setArgCount(2);
  amap.registerOption("--navs", "Generate navs, just to see if it works");


  auto argCode = amap.parse(argc, argv);
  if (argCode == ArgMap::Error) {
    return -1;
  }
  if (argCode == ArgMap::Done) {
    return 0;
  }

  NKEParser parser;
  NKEData data = parser.load(filename);

  LOG(INFO) << "Successfully loaded CSV file.";
  LOG(INFO) << "Loaded a table with " << data.rows() << " rows and " << data.cols() << " columns.";

  if (amap.optionProvided("--angle")) {
    int i = 0, j = 0;
    readIJ(amap, "--angle", &i, &j);
    std::cout << "Angle: " << data.col(j).angle(i) << std::endl;
  }

  if (amap.optionProvided("--duration")) {
    int i = 0, j = 0;
    readIJ(amap, "--duration", &i, &j);
    std::cout << "Duration: " << data.col(j).duration(i) << std::endl;
  }

  if (amap.optionProvided("--velocity")) {
    int i = 0, j = 0;
    readIJ(amap, "--velocity", &i, &j);
    std::cout << "Velocity: " << data.col(j).velocity(i) << std::endl;
  }

  if (amap.optionProvided("--navs")) {
    NavCollection navs = parser.makeNavs(Nav::debuggingBoatId(), data);
    LOG(INFO) << "Produced " << navs.size() << " navs.";
  }

  if (amap.optionProvided("--ncc")) {
    auto args = amap.optionArgs("--ncc");
    NKEArray X = data.getByType(parser.type(args[0]->value()));
    NKEArray Y = data.getByType(parser.type(args[1]->value()));
    std::cout << "ncc = " << ncc(X, Y) << std::endl;
  }




  LOG(INFO) << "Success";
  return 0;
}
