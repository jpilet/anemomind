/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GEOGRAPHICPOSITIONJSON_H_
#define GEOGRAPHICPOSITIONJSON_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
template <typename T> class GeographicPosition;
typedef GeographicPosition<double> GeoPosd;
namespace json {

Poco::Dynamic::Var serialize(const GeoPosd &posd);
bool deserialize(Poco::Dynamic::Var src, GeoPosd *dst);

}
} /* namespace sail */

#endif /* GEOGRAPHICPOSITIONJSON_H_ */
