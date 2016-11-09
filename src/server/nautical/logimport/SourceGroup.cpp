/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include <server/nautical/logimport/SourceGroup.h>
#include <server/nautical/logimport/LogAccumulator.h>

namespace sail {

SourceGroup::SourceGroup() :
  awa(nullptr), aws(nullptr), twa(nullptr),
  tws(nullptr), magHdg(nullptr), geoPos(nullptr),
  gpsBearing(nullptr), gpsSpeed(nullptr), watSpeed(nullptr) {}

SourceGroup::SourceGroup(const std::string &srcName, LogAccumulator *dst) :
    awa(allocateSourceIfNeeded<Angle<double> >(srcName, dst->getAWAsources())),
    aws(allocateSourceIfNeeded<Velocity<double> >(srcName, dst->getAWSsources())),
    twa(allocateSourceIfNeeded<Angle<double> >(srcName, dst->getTWAsources())),
    tws(allocateSourceIfNeeded<Velocity<double> >(srcName, dst->getTWSsources())),
    magHdg(allocateSourceIfNeeded<Angle<double> >(srcName, dst->getMAG_HEADINGsources())),
    watSpeed(allocateSourceIfNeeded<Velocity<double> >(srcName, dst->getWAT_SPEEDsources())),
    geoPos(allocateSourceIfNeeded<GeographicPosition<double> >(srcName, dst->getGPS_POSsources())),
    gpsBearing(allocateSourceIfNeeded<Angle<double> >(srcName, dst->getGPS_BEARINGsources())),
    gpsSpeed(allocateSourceIfNeeded<Velocity<double> >(srcName, dst->getGPS_SPEEDsources()))
  {}

}
