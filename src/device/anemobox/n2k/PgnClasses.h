/** Generated on Thu Jan 21 2016 16:27:49 GMT+0100 (CET) using 
 *
 *     node /home/jonas/programmering/sailsmart/src/device/anemobox/n2k/codegen/index.js /home/jonas/programmering/cpp/canboat/analyzer/pgns.xml
 *
 *  WARNING: Modifications to this file will be overwritten when it is re-generated
 */
#ifndef _PGNCLASSES_HEADER_
#define _PGNCLASSES_HEADER_ 1

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <cassert>
#include <server/common/Optional.h>

namespace PgnClasses {

  class WindData {
  public:
    static const int pgn = 130306;

    WindData();
    WindData(uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {assert(_valid); return _sid;}
    const Optional<sail::Velocity<double> > &windSpeed() const {assert(_valid); return _windSpeed;}
    const Optional<sail::Angle<double> > &windAngle() const {assert(_valid); return _windAngle;}
    const Optional<uint64_t > &reference() const {assert(_valid); return _reference;}
  private:
    bool _valid;
    // Number of fields: 4
    Optional<uint64_t > _sid;
    Optional<sail::Velocity<double> > _windSpeed;
    Optional<sail::Angle<double> > _windAngle;
    Optional<uint64_t > _reference;
  };
}

#endif