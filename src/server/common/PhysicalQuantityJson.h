/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Json interface for Physical quantities
 */

#ifndef PHYSICALQUANTITYJSON_H_
#define PHYSICALQUANTITYJSON_H_

#include <Poco/JSON/Object.h>
#include <server/common/PhysicalQuantity.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

#define MAKE_PHYSQUANT_JSON_INTERFACE_H(TypeName) \
    void readField(Poco::JSON::Object::Ptr obj, std::string fieldPrefix, TypeName<double> *out, bool require = true); \
    void writeField(Poco::JSON::Object::Ptr obj, std::string fieldPrefix, const TypeName<double> &x);

MAKE_PHYSQUANT_JSON_INTERFACE_H(Duration)
MAKE_PHYSQUANT_JSON_INTERFACE_H(Velocity)
MAKE_PHYSQUANT_JSON_INTERFACE_H(Angle)
MAKE_PHYSQUANT_JSON_INTERFACE_H(Length)
MAKE_PHYSQUANT_JSON_INTERFACE_H(Mass)

#undef MAKE_PHYSQUANT_JSON_INTERFACE_H

}
} /* namespace sail */

#endif /* PHYSICALQUANTITYJSON_H_ */
