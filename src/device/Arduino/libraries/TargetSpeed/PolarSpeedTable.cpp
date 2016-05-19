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
  if (_file) {
    _file.close();
  }
  _twsCount = 0;
  _twaCount = 0;
}

bool PolarSpeedTable::load(const char *filename) {
  _file = SD.open(filename);

  /*    _twsStep        sizeof(PolarSpeedTable::FixType)
   *    _twsCount       sizeof(unsigned char)
   *    _twaCount       sizeof(unsigned char)
   */
  if (bool(_file)) {
    {
      FixType twsStep;
      readFixedPoint(&twsStep, &_file);
      _twsStep = Velocity<FixType>::knots(twsStep);
    }
    _twsCount = readInteger<unsigned char>(&_file);
    _twaCount = readInteger<unsigned char>(&_file);
    if (_twsCount != 0 && _twaCount != 0) {
      _twaStep = Angle<FixType>::degrees(FixType(360)/FixType(_twaCount));
      return true;
    }
  }
  invalidate();
  return false;
}

PolarSpeedTable::~PolarSpeedTable() {
  invalidate();
}

Velocity<PolarSpeedTable::FixType> PolarSpeedTable::targetSpeed(Velocity<PolarSpeedTable::FixType> tws,
    Angle<PolarSpeedTable::FixType> twa) {

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
    Angle<PolarSpeedTable::FixType> twa) {
  if (empty() || twsIndex == 0) {
    return Velocity<PolarSpeedTable::FixType>::knots(PolarSpeedTable::FixType(0));
  }
  PolarSpeedTable::FixType twaRealIndex = (twa.positiveMinAngle()/_twaStep).getScalar();
  int twaIndex = int(twaRealIndex);

  PolarSpeedTable::FixType lambda = twaRealIndex - PolarSpeedTable::FixType(twaIndex);

  return (FixType(1) - lambda)*get(twsIndex, twaIndex)
      + lambda*get(twsIndex, (twaIndex + 1) % _twaCount);
}

PolarSpeedTable::FixType PolarSpeedTable::calcTwaRealIndex(Angle<PolarSpeedTable::FixType> twa) const {
  return (twa.positiveMinAngle()/_twaStep).getScalar();
}

PolarSpeedTable::FixType PolarSpeedTable::calcTwsRealIndex(Velocity<PolarSpeedTable::FixType> tws) const {
  return (tws/_twsStep).getScalar();
}

int PolarSpeedTable::tableIndex(int twsIndex, int twaIndex) const {
  /* A tws=0 knots always map to twsIndex=0 and there is no use storing this in
   * the table, since the target speed will always be 0 if there is no wind.
   * Therefore, twsIndex=1 should map to the first table (twsIndex-1 = 1-1 = 0)
   * of target speeds for various values of twaIndex.
   */
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
  _file.seek(tableEntryFilePos(twsIndex, twaIndex));
  FixType raw;
  readFixedPoint(&raw, &_file);
  return Velocity<FixType>::knots(raw);
}


} /* namespace mmm */
