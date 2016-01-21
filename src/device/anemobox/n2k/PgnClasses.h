/** Generated on Thu Jan 21 2016 12:37:30 GMT+0100 (CET) using 
 *
 *  node codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml /home/jonas/programmering/sailsmart/src/device/anemobox/n2k
 *
 */
#ifndef _PGNCLASSES_HEADER_
#define _PGNCLASSES_HEADER_ 1

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <cassert>
class BitStream;

namespace PgnClasses {

  class WindData {
  public:
    WindData();
    WindData(BitStream *src);
    bool valid() const {return _valid;}

    // Field access
    const int64_t &sid() const {assert(_valid); return _sid;}
    const sail::Velocity<double> &windSpeed() const {assert(_valid); return _windSpeed;}
    const sail::Angle<double> &windAngle() const {assert(_valid); return _windAngle;}
    const int64_t &reference() const {assert(_valid); return _reference;}
  private:
    bool _valid;
    // Number of fields: 4
    int64_t _sid;
    sail::Velocity<double> _windSpeed;
    sail::Angle<double> _windAngle;
    int64_t _reference;
  };
}

#endif