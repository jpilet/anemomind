/** Generated on Thu Jan 21 2016 11:47:21 GMT+0100 (CET) using 
 *
 *  node codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml /home/jonas/programmering/sailsmart/src/device/anemobox/n2k
 *
 */
#ifndef _PGNCLASSES_HEADER_
#define _PGNCLASSES_HEADER_ 1

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
class BitStream;

namespace PgnClasses {
  class WindData {
  public:
    WindData(BitStream *src);

    // Field access
    const int64_t &getSid() const {return _sid;}
    const Velocity<double> &getWindSpeed() const {return _windSpeed;}
    const Angle<double> &getWindAngle() const {return _windAngle;}
    const int64_t &getReference() const {return _reference;}
  private:
    // Number of fields: 4
    int64_t _sid;
    Velocity<double> _windSpeed;
    Angle<double> _windAngle;
    int64_t _reference;
  };
}

#endif