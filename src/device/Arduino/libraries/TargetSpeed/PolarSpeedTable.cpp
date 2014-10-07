/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PolarSpeedTable.h"

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
  _file(fopen(filename, "rb")) {
/*       *    _twsStep        sizeof(PolarSpeedTable::FixType)
       *    _twaStep        sizeof(PolarSpeedTable::FixType)
       *    _twsCount       sizeof(unsigned char)
       *    _twaCount       sizeof(unsigned char)*/
  if (_file == nullptr) {
    invalidate();
  } else {
    {
      FixType twsStep;
      freadFixedPoint(&twsStep, _file);
      _twsStep = Velocity<FixType>::knots(twsStep);
    }
    _twsCount = freadInteger<unsigned char>(_file);
    _twaCount = freadInteger<unsigned char>(_file);
    _twaStep = Angle<FixType>::degrees(FixType(360)/FixType(_twaCount));
  }
}

PolarSpeedTable::~PolarSpeedTable() {
  if (_file != nullptr) {
    fclose(_file);
  }
}

Velocity<PolarSpeedTable::FixType> PolarSpeedTable::targetSpeed(Velocity<PolarSpeedTable::FixType> tws,
    Angle<PolarSpeedTable::FixType> twa) const {

    if (empty()) {
      return Velocity<FixType>::knots(FixType(-1));
    }

    PolarSpeedTable::FixType twsRealIndex = calcTwsRealIndex(tws);
    int twsIndex = int(twsRealIndex);

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
    Angle<PolarSpeedTable::FixType> twa) const {
  if (twsIndex == 0) {
    return Velocity<PolarSpeedTable::FixType>::knots(PolarSpeedTable::FixType(0));
  }
  PolarSpeedTable::FixType twaRealIndex = twa.positiveMinAngle()/_twaStep;
  int twaIndex = int(twaRealIndex);
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

Velocity<PolarSpeedTable::FixType> PolarSpeedTable::get(int twsIndex, int twaIndex) const {
  // It is assumed that _file is open and can be read.
  fseek(_file, tableEntryFilePos(twsIndex, twaIndex), SEEK_SET);
  FixType raw;
  freadFixedPoint(&raw, _file);
  return Velocity<FixType>::knots(raw);
}


PolarSpeedTable::PolarSpeedTable(const PolarSpeedTable &other) :
  _twsCount(-1), _twaCount(-1), _file(nullptr) {}

void PolarSpeedTable::operator= (const PolarSpeedTable &other) {}


} /* namespace mmm */
