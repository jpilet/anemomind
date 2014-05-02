/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMPJSON_H_
#define TIMESTAMPJSON_H_

#include <string>
#include <Poco/JSON/Object.h>

namespace sail {

class TimeStamp;

namespace json {

bool deserializeField(Poco::JSON::Object::Ptr obj, const std::string &prefix, TimeStamp *out);
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &prefix, const TimeStamp &x);

Poco::JSON::Object::Ptr serialize(const TimeStamp &src);
bool deserialize(Poco::JSON::Object::Ptr src, TimeStamp *dst);

}
} /* namespace sail */

#endif /* TIMESTAMPJSON_H_ */
