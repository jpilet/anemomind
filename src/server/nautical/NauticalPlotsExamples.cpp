/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NauticalPlotsExamples.h"
#include "NauticalPlots.h"
#include <iostream>

namespace sail {

void npex001() {
  Array<Nav> navs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      navs, Duration<>::minutes(10).seconds());

  plotPolarAWAAndWatSpeed(splitNavs);
}

void npex002() {
  Array<Nav> allnavs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      allnavs, Duration<>::minutes(10).seconds());


  int i = 2;

  int binCount = 60;
  Array<Nav> navs = splitNavs[i];
  navs = navs.slice(makeReliableNavMask(navs));

  PeriodicHist watSpeedSum = makeAWAWatSpeedSumHist(navs, binCount);
  PeriodicHist awsSum = makeAWAAWSSumHist(navs, binCount);
  PeriodicHist awa = makeAWAHist(navs, binCount);

  //plotHist(awa);
  //plotHist(calcAverage(watSpeedSum, awa));
  plotHist(calcAverage(awsSum, awa));

}

void npex003() {
  Array<Nav> allnavs = loadNavsFromText(Nav::AllNavsPath, false);
  Array<Array<Nav> > splitNavs = splitNavsByDuration(
      allnavs, Duration<>::minutes(10).seconds());


  int binCount = 60;

  for (int i = 0; i < splitNavs.size(); i++) {
    Array<Nav> navs = splitNavs[i];
    navs = navs.slice(makeReliableNavMask(navs));

    PeriodicHist watSpeedSum = makeAWAWatSpeedSumHist(navs, binCount);
    PeriodicHist awsSum = makeAWAAWSSumHist(navs, binCount);
    PeriodicHist awa = makeAWAHist(navs, binCount);

    PeriodicHist avg = calcAverage(awsSum, awa);
    int mini = avg.minBin();
    std::cout << "Navs " << i << " Min bin with middle at " << avg.indexer().binMiddle(mini).degrees() << " degrees." << std::endl;
  }
}

}
