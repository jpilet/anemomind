/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef LINEKMJSON_H_
#define LINEKMJSON_H_

#include <Poco/JSON/Object.h>

namespace sail {
class LineKM;
namespace json {
Poco::JSON::Object::Ptr serialize(const LineKM &x);
void deserialize(Poco::JSON::Object::Ptr src, LineKM *dst);

}
} /* namespace sail */

#endif /* LINEKMJSON_H_ */
