/*
 *  Created on: 2014-03-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TIMESTAMPJSON_H_
#define TIMESTAMPJSON_H_

#include <string>
#include <Poco/JSON/Object.h>
#include <server/common/Json.h>


namespace sail {

class TimeStamp;

namespace json {

bool deserializeField(Poco::Dynamic::Var obj, const std::string &prefix, TimeStamp *out);
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &prefix, const TimeStamp &x);

Poco::Dynamic::Var serialize(const TimeStamp &src);
bool deserialize(Poco::Dynamic::Var src, TimeStamp *dst);

}
} /* namespace sail */

#endif /* TIMESTAMPJSON_H_ */
