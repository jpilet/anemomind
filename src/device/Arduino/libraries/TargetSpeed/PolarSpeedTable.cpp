/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PolarSpeedTable.h"


#include <iostream>
#include <server/common/string.h>
#include <assert.h>

namespace sail {

PolarSpeedTable::PolarSpeedTable() {
  invalidate();
}

void PolarSpeedTable::invalidate() {
  _file = nullptr;
  _twsCount = -1;
  _twaCount = -1;
}


PolarSpeedTable::PolarSpeedTable(const char *filename) :
  _file(SD.open(filename)) {
/*       *    _twsStep        sizeof(PolarSpeedTable::FixType)
       *    _twaStep        sizeof(PolarSpeedTable::FixType)
       *    _twsCount       sizeof(unsigned char)
       *    _twaCount       sizeof(unsigned char)*/
  if (bool(_file)) {
    {
      FixType twsStep;
      readFixedPoint(&twsStep, &_file);
      _twsStep = Velocity<FixType>::knots(twsStep);
    }
    _twsCount = readInteger<unsigned char>(&_file);
    _twaCount = readInteger<unsigned char>(&_file);
    _twaStep = Angle<FixType>::degrees(FixType(360)/FixType(_twaCount));

    std::cout << "tws-step = " << double(_twsStep.knots()) << std::endl;
    std::cout << "twa-step = " << double(_twaStep.degrees()) << std::endl;


  } else {
    _file = File();
  }
}

PolarSpeedTable::~PolarSpeedTable() {
  _file.close();
}

Velocity<PolarSpeedTable::FixType> PolarSpeedTable::targetSpeed(Velocity<PolarSpeedTable::FixType> tws,
    Angle<PolarSpeedTable::FixType> twa) {

    if (empty()) {
      return Velocity<FixType>::knots(FixType(-1));
    }

    PolarSpeedTable::FixType twsRealIndex = calcTwsRealIndex(tws);
    int twsIndex = int(twsRealIndex);
    std::cout << EXPR_AND_VAL_AS_STRING(twsIndex) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(double(twsRealIndex)) << std::endl;

    // Check if we are at or below the lower limit of the tws table
    if (twsIndex < 0) {
      return Velocity<PolarSpeedTable::FixType>::knots(PolarSpeedTable::FixType(0));
    }

    // Check if we are at or above the upper limit of the tws table.
    if (twsIndex >= _twsCount) {
      return targetSpeedForTwsIndex(twsIndex, twa);
    }

    // Linear interpolation
    PolarSpeedTable::FixType lambda = twsRealIndex - PolarSpeedTable::FixType(twsIndex);

    return (FixType(1) - lambda)*targetSpeedForTwsIndex(twsIndex + 0, twa)
                 + lambda*targetSpeedForTwsIndex(twsIndex + 1, twa);
}


Velocity<PolarSpeedTable::FixType> PolarSpeedTable::targetSpeedForTwsIndex(int twsIndex,
    Angle<PolarSpeedTable::FixType> twa) {
  if (twsIndex == 0) {
    return Velocity<PolarSpeedTable::FixType>::knots(PolarSpeedTable::FixType(0));
  }
  PolarSpeedTable::FixType twaRealIndex = twa.positiveMinAngle()/_twaStep;
  int twaIndex = int(twaRealIndex);
  std::cout << EXPR_AND_VAL_AS_STRING(twaIndex) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(double(twaRealIndex)) << std::endl;

  PolarSpeedTable::FixType lambda = twaRealIndex - PolarSpeedTable::FixType(twaIndex);

  return (FixType(1) - lambda)*get(twsIndex, twaIndex)
      + lambda*get(twsIndex, (twaIndex + 1) % _twaCount);
}

PolarSpeedTable::FixType PolarSpeedTable::calcTwaRealIndex(Angle<PolarSpeedTable::FixType> twa) const {
  return twa.positiveMinAngle()/_twaStep;
}

PolarSpeedTable::FixType PolarSpeedTable::calcTwsRealIndex(Velocity<PolarSpeedTable::FixType> tws) const {
  return tws/_twsStep;
}

int PolarSpeedTable::tableIndex(int twsIndex, int twaIndex) const {
  return (twsIndex - 1)*_twaCount + twaIndex;
}

int PolarSpeedTable::tableEntryFilePos(int twsIndex, int twaIndex) const {
  return FILE_HEADER_SIZE + tableIndex(twsIndex, twaIndex)*TABLE_ENTRY_SIZE;
}

int PolarSpeedTable::fileSize() const {
  return FILE_HEADER_SIZE + _twsCount*_twaCount*sizeof(FixType);
}

Velocity<PolarSpeedTable::FixType> PolarSpeedTable::get(int twsIndex, int twaIndex) {
  // It is assumed that _file is open and can be read.
  assert(_file.seek(tableEntryFilePos(twsIndex, twaIndex)));
  FixType raw;
  readFixedPoint(&raw, &_file);
  std::cout << "   At (" << twsIndex << ", " << twaIndex << "): " << double(raw) << std::endl;
  return Velocity<FixType>::knots(raw);
}

PolarSpeedTable::PolarSpeedTable(const PolarSpeedTable &other) :
  _twsCount(-1), _twaCount(-1), _file(nullptr) {}

void PolarSpeedTable::operator= (const PolarSpeedTable &other) {}


} /* namespace mmm */
