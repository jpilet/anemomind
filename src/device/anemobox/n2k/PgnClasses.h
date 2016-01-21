/** Generated on Thu Jan 21 2016 20:41:35 GMT+0100 (CET) using 
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

  class VesselHeading {
  public:
    static const int pgn = 127250;

    VesselHeading();
    VesselHeading(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {assert(_valid); return _sid;}
    const Optional<sail::Angle<double> > &heading() const {assert(_valid); return _heading;}
    const Optional<sail::Angle<double> > &deviation() const {assert(_valid); return _deviation;}
    const Optional<sail::Angle<double> > &variation() const {assert(_valid); return _variation;}
    const Optional<uint64_t > &reference() const {assert(_valid); return _reference;}
  private:
    bool _valid;
    // Number of fields: 5
    Optional<uint64_t > _sid;
    Optional<sail::Angle<double> > _heading;
    Optional<sail::Angle<double> > _deviation;
    Optional<sail::Angle<double> > _variation;
    Optional<uint64_t > _reference;
  };

  class Attitude {
  public:
    static const int pgn = 127257;

    Attitude();
    Attitude(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {assert(_valid); return _sid;}
  private:
    bool _valid;
    // Number of fields: 4
    Optional<uint64_t > _sid;
  };

  class Speed {
  public:
    static const int pgn = 128259;

    Speed();
    Speed(const uint8_t *data, int lengthBytes);
    bool valid() const {return _valid;}
    void reset();

    // Field access
    const Optional<uint64_t > &sid() const {assert(_valid); return _sid;}
    const Optional<sail::Velocity<double> > &speedWaterReferenced() const {assert(_valid); return _speedWaterReferenced;}
    const Optional<sail::Velocity<double> > &speedGroundReferenced() const {assert(_valid); return _speedGroundReferenced;}
    const Optional<uint64_t > &speedWaterReferencedType() const {assert(_valid); return _speedWaterReferencedType;}
  private:
    bool _valid;
    // Number of fields: 4
    Optional<uint64_t > _sid;
    Optional<sail::Velocity<double> > _speedWaterReferenced;
    Optional<sail::Velocity<double> > _speedGroundReferenced;
    Optional<uint64_t > _speedWaterReferencedType;
  };

  class WindData {
  public:
    static const int pgn = 130306;

    WindData();
    WindData(const uint8_t *data, int lengthBytes);
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

class PgnVisitor {
 public:
  bool visit(int pgn, const uint8_t *data, int length);
 protected:
  virtual bool apply(const VesselHeading& packet) { return false; }
  virtual bool apply(const Attitude& packet) { return false; }
  virtual bool apply(const Speed& packet) { return false; }
  virtual bool apply(const WindData& packet) { return false; }
};

}

#endif