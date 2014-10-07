/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARSPEEDTABLE_H_
#define POLARSPEEDTABLE_H_

#include "../FixedPoint/FixedPoint.h"
#include "../PhysicalQuantity/PhysicalQuantity.h"
#include "../Endian/EndianIO.h"
#include <cstdio>

namespace sail {

class PolarSpeedTable {
 public:
  typedef FP8_8 FixType;
  /*
   * File format:
   *
   *  Header:
   *    _twsStep        sizeof(FixType)
   *    _twaStep        sizeof(FixType)
   *    _twsCount       sizeof(unsigned char)
   *    _twaCount       sizeof(unsigned char)
   *
   *  Body: _twsCount*_twaCount entries of type
   *    targetSpeed     sizeof(FixType)
   *
   */
  static constexpr int FILE_HEADER_SIZE = 2*sizeof(FixType)           // _twsStep, _twaStep
                                        + 2*sizeof(unsigned char);  // _twsCount, _twaCount
  static constexpr int TABLE_ENTRY_SIZE = sizeof(FixType);

  PolarSpeedTable();
  PolarSpeedTable(const char *filename);
  ~PolarSpeedTable();

  Velocity<FixType> targetSpeed(Velocity<FixType> tws, Angle<FixType> twa) const;

  Velocity<FixType> maxTws() const {return FixType(_twsCount)*_twsStep;}
  Velocity<FixType> minTws() const {return Velocity<FixType>::knots(FixType(0));}
  bool empty() const {return _file == nullptr;}




  /*
   * Use this static function to create a file containing a table.
   */
  template <typename TargetSpeedFunction>
    /*
     * TargetSpeedFunction is an object that has a method
     *   Velocity<PolarSpeedTable>::FixType operator()
     *      (Velocity<PolarSpeedTable::FixType> tws, Angle<PolarSpeedTable::FixType> twa);
     *
     *   that specifies the target speed for any tws/twa.
     */
  static bool build(Velocity<FixType> twsStep, Angle<FixType> twaStep,
    unsigned char twsCount, unsigned char twaCount,
    TargetSpeedFunction fun,
    const char *dstFilename) {
    FILE *file = fopen(dstFilename, "wb");
    if (file == nullptr) {
      return false;
    }
    fwriteFixedPoint(twsStep.knots(), file);
    fwriteFixedPoint(twaStep.degrees(), file);
    fwriteInteger(twsCount, file);
    fwriteInteger(twaCount, file);

    for (int twsIndex = 1; twsIndex <= twsCount; twsIndex++) {
      for (int twaIndex = 0; twaIndex < twaCount; twaIndex++) {
        fwriteFixedPoint(fun(FixType(twsIndex)*twsStep,
            FixType(twaIndex)*twaStep), file);
      }
    }
    fclose(file);

    return true;
  }

 private:
  void invalidate();
  int tableIndex(int twsIndex, int twaIndex) const;
  int tableEntryFilePos(int twsIndex, int twaIndex) const;
  int fileSize() const;
  Velocity<FixType> get(int twsIndex, int twaIndex) const;

  // Prohibit copying and assignment. If we were to support these operations,
  // we would need some form of reference counting or something for the file pointer.
  PolarSpeedTable(const PolarSpeedTable &other);
  void operator= (const PolarSpeedTable &other);

  Velocity<FixType> targetSpeedForTwsIndex(int twsIndex,
      Angle<FixType> twa) const;

  FixType calcTwaRealIndex(Angle<FixType> twa) const;
  FixType calcTwsRealIndex(Velocity<FixType> tws) const;

  FILE *_file;
  unsigned char _twsCount, _twaCount;
  Angle<FixType> _twaStep;
  Velocity<FixType> _twsStep;
};

}

#endif /* POLARSPEEDTABLE_H_ */
