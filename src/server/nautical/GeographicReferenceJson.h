/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GEOGRAPHICREFERENCEJSON_H_
#define GEOGRAPHICREFERENCEJSON_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
class GeographicReference;
namespace json {

Poco::Dynamic::Var serialize(const GeographicReference &geoRef);
bool deserialize(Poco::Dynamic::Var src, GeographicReference *dst);

}
} /* namespace sail */

#endif /* GEOGRAPHICREFERENCEJSON_H_ */
