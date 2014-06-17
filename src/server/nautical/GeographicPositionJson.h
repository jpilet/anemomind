/*
 *  Created on: Jun 17, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GEOGRAPHICPOSITIONJSON_H_
#define GEOGRAPHICPOSITIONJSON_H_

namespace sail {
template <typename T> class GeographicPosition;
typedef GeographicPosition<double> GeoPosd;
namespace json {

Poco::Dynamic::Var serialize(const GeoPosd &posd);
void deserialize(Poco::Dynamic::Var src, GeoPosd *dst);

}
} /* namespace sail */

#endif /* GEOGRAPHICPOSITIONJSON_H_ */
