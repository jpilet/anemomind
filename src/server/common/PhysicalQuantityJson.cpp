/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "PhysicalQuantityJson.h"
#include <assert.h>

namespace sail {
namespace json {

// I think there is no beautiful generic way of calling the appropriate Unit() method
// for different types.
// I think we have to resort to macros here :-(
#define MAKE_PHYSQUANT_JSON_INTERFACE(TypeName, Unit, Suffix) \
    void readField(Poco::JSON::Object::Ptr obj, std::string fieldPrefix, TypeName<double> *out, bool require) { \
      std::string fname = fieldPrefix + (Suffix); \
      bool is = obj->has(fname); \
      if (is) { \
        *out = TypeName<double>::Unit(obj->getValue<double>(fname)); \
      } else if (require) { \
        throw MissingFieldException(obj, fname); \
      } else { \
        *out = TypeName<double>(); \
      } \
    } \
    void writeField(Poco::JSON::Object::Ptr obj, std::string fieldPrefix, const TypeName<double> &x) { \
      double val = x.Unit(); \
      if (!std::isnan(val)) { \
        obj->set(fieldPrefix + (Suffix), x.Unit()); \
      } \
    }

MAKE_PHYSQUANT_JSON_INTERFACE(Duration, seconds, "-s");
MAKE_PHYSQUANT_JSON_INTERFACE(Velocity, metersPerSecond, "-mps");
MAKE_PHYSQUANT_JSON_INTERFACE(Angle, radians, "-rad");
MAKE_PHYSQUANT_JSON_INTERFACE(Length, meters, "-m");
MAKE_PHYSQUANT_JSON_INTERFACE(Mass, kilograms, "-kg");

#undef MAKE_PHYSQUANT_JSON_INTERFACE

}
} /* namespace sail */
