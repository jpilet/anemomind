/*
 *  Created on: 2014-10-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef POLARSPEEDTABLE_H_
#define POLARSPEEDTABLE_H_

#include "../Endian/Endian.h"

#ifndef ON_SERVER
  #include <SD.h>
#else
  // For the static 'build' method.
  #include <iostream>
  #include "../Endian/EndianIOStream.h"

  // An object that exhibits the same interface as the SD object of Arduino
  #include <device/Arduino/NMEAStats/test/MockArduino/SD.h>
#endif

#include "../Endian/EndianIOSD.h"
#include "../PhysicalQuantity/PhysicalQuantity.h"

#include <iostream>
#include <cassert>

namespace sail {


template <typename X, typename Y, int n>
std::ostream &operator << (std::ostream &s, FixedPoint<X, Y, n> x) {
  s << double(x) << "[fixpt]";
  return s;
}

template <typename X, typename Y, int n>
std::ostream &operator << (std::ostream &s, Velocity<FixedPoint<X, Y, n> > x) {
  s << double(x.knots()) << " knots[fixpt]";
  return s;
}

template <typename X, typename Y, int n>
std::ostream &operator << (std::ostream &s, Angle<FixedPoint<X, Y, n> > x) {
  s << double(x.degrees()) << " degrees[fixpt]";
  return s;
}

inline std::ostream &operator << (std::ostream &s, unsigned char x) {
  s << int(x);
  return s;
}



template <typename T>
const T &dr(const T &x, const char *xs, const char *file, int line) {
  std::cout << "File " << file << " on line " << line << ": " << xs << " = [[[" << x << "]]]" << std::endl;
  return x;
}

#define DR(x) dr(x, #x, __FILE__, __LINE__)


class PolarSpeedTable {
 public:
  // Used to approximate real numbers in fixed point representation.
  typedef FP16_16 FixType;

  /*
   * File format:
   *
   *  Header:
   *    _twsStep        sizeof(FixType)
   *    _twsCount       sizeof(unsigned char)
   *    _twaCount       sizeof(unsigned char)
   *
   *  Body: _twsCount*_twaCount entries of type
   *    targetSpeed     sizeof(FixType)
   *
   */
  static const int FILE_HEADER_SIZE = sizeof(FixType)           // _twsStep
                                        + 2*sizeof(unsigned char);  // _twsCount, _twaCount
  static const int TABLE_ENTRY_SIZE = sizeof(FixType);

  PolarSpeedTable();
  ~PolarSpeedTable();

  // Prohibit copying and assignment. If we were to support these operations,
  // we would need some form of reference counting or something for the file pointer.
  PolarSpeedTable(const PolarSpeedTable &other) = delete;
  void operator= (const PolarSpeedTable &other) = delete;

  bool load(const char *filename);

  Velocity<FixType> targetSpeed(Velocity<FixType> tws, Angle<FixType> twa);

  Velocity<FixType> maxTws() const {return FixType(_twsCount)*_twsStep;}
  Velocity<FixType> minTws() const {return Velocity<FixType>::knots(FixType(0));}
  bool empty() {return !bool(_file) || _twsCount == 0;}




  #ifdef ON_SERVER
  /*
   * Use this static function to create a file containing a table.
   * Since it will most likely be used only on the server, we can
   * allow ourselves to use double here.
   */
  template <typename TargetSpeedFunction>
    /*
     * TargetSpeedFunction is an object that has a method
     *
     *   Velocity<double> operator() (Velocity<double> tws, Angle<double> twa);
     *
     *   that specifies the target speed for any tws/twa.
     */
  static bool build(Velocity<double> twsStep,
    int twsCount, int twaCount,
    TargetSpeedFunction fun,
    std::ostream *out) {

    if (twsCount < 0 || 255 < twsCount || twaCount < 0 || 255 < twaCount) {
      return false;
    }

    writeBinaryFixedPoint(DR(FixType(twsStep.knots())), out);
    writeBinaryInteger(DR((unsigned char)(twsCount)), out);
    writeBinaryInteger(DR((unsigned char)(twaCount)), out);

    Angle<double> twaStep = (1.0/twaCount)*Angle<double>::degrees(360.0);

    for (int twsIndex = 1; twsIndex <= twsCount; twsIndex++) {
      for (int twaIndex = 0; twaIndex < twaCount; twaIndex++) {
        writeBinaryFixedPoint(
            FixType(fun(double(twsIndex)*twsStep,
                double(twaIndex)*twaStep).knots()), out);
      }
    }

    return out->good();
  }
  #endif
 private:
  void invalidate();
  int tableIndex(int twsIndex, int twaIndex) const;
  int tableEntryFilePos(int twsIndex, int twaIndex) const;
  int fileSize() const;
  Velocity<FixType> get(int twsIndex, int twaIndex);

  Velocity<FixType> targetSpeedForTwsIndex(int twsIndex,
      Angle<FixType> twa);

  FixType calcTwaRealIndex(Angle<FixType> twa) const;
  FixType calcTwsRealIndex(Velocity<FixType> tws) const;

  File _file;
  unsigned char _twsCount, _twaCount;
  Angle<FixType> _twaStep;
  Velocity<FixType> _twsStep;
};

}

#endif /* POLARSPEEDTABLE_H_ */
