/*
 *  Created on: 28 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMPJSON_H_
#define TIMESTAMPJSON_H_

#include <server/common/Json.h>

namespace sail {

class TimeStamp;

namespace json {

bool serializeField(Poco::JSON::Object::Ptr obj, std::string prefix, TimeStamp *out);
void deserializeField(Poco::JSON::Object::Ptr obj, std::string prefix, const TimeStamp &x);

}
} /* namespace sail */

#endif /* TIMESTAMPJSON_H_ */
