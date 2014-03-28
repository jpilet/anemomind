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
namespace json {



Poco::JSON::Object::Ptr encode(const Nav &nav);
void decode(Poco::JSON::Object::Ptr x, Nav *out);

Poco::JSON::Array encode(Array<Nav> nav);
void decode(Poco::JSON::Array src, Array<Nav> *dst);

// TODO: decode

}
} /* namespace sail */

#endif /* NAVJSON_H_ */
