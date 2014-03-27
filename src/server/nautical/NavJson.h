/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Conversion from/to Json for the Nav datatype.
 */

#ifndef NAVJSON_H_
#define NAVJSON_H_

#include <Poco/JSON/Object.h>
#include <server/nautical/Nav.h>

namespace sail {

Poco::JSON::Object::Ptr convertToJson(Array<Nav> navs);

} /* namespace sail */

#endif /* NAVJSON_H_ */
