/*
 *  Created on: 2014-03-12
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "NauticalPlotsExamples.h"
#include "NauticalPlots.h"

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


  int binCount = 60;
  Array<Nav> navs = splitNavs[0];
  navs = navs.slice(makeReliableNavMask(navs));

  PeriodicHist watSpeedSum = makeAWAWatSpeedSumHist(navs, binCount);
  PeriodicHist awsSum = makeAWAAWSSumHist(navs, binCount);
  PeriodicHist awa = makeAWAHist(navs, binCount);

  //plotHist(calcAverage(watSpeedSum, awa));
  plotHist(awa);
  //plotHist(calcAverage(awsSum, awa));

}

//void npex003() {
//  Array<Nav> allnavs = loadNavsFromText(Nav::AllNavsPath, false);
//  Array<Array<Nav> > splitNavs = splitNavsByDuration(
//      allnavs, Duration<>::minutes(10).seconds());
//
//
//  int binCount = 60;
//
//
//  Array<Nav> navs = splitNavs[0];
//  na
//
//  PeriodicHist watSpeedSum = makeAWAWatSpeedSumHist(navs, binCount);
//  PeriodicHist awsSum = makeAWAAWSSumHist(navs, binCount);
//  PeriodicHist awa = makeAWAHist(navs, binCount);
//
//  plotHist(calcAverage(watSpeedSum, awa));
//  //plotHist(awa);
//  //plotHist(calcAverage(awsSum, awa));
//
//}

}
