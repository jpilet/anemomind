/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSONFWD_H_
#define JSONFWD_H_

namespace sail {
namespace json {

template <typename T>
Poco::Dynamic::Var toJsonObjectWithField(const std::string &typeName, const T &x);

template <typename T>
Poco::Dynamic::Var serializeArray(Array<T> src);

template <typename T>
Poco::Dynamic::Var serialize(Array<T> src, std::function<Poco::Dynamic::Var(T)> customSerializer);

template <typename T>
Poco::Dynamic::Var serialize(Array<T> src);

template <typename T>
bool deserialize(Poco::Dynamic::Var csrc, Array<T> *dst);

void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value);

bool deserializeField(Poco::Dynamic::Var obj, const std::string &fieldName, std::string *valueOut);

}
}

#endif /* JSONFWD_H_ */
