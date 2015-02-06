/*
 *  Created on: 2014-04-01
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 * The implementations of these functions are defined in Json.impl.h.
 * Including this file from a .h is fine.
 * Including this file from a .cpp requires including Json.impl.h after all
 * other headers.
 *
 * Otherwise, you may get an error message such as:
 * src/server/nautical/NavJson.h:17:20: note:
 *     'serialize' should be declared prior to the call site or in namespace 'sail'
 */

#ifndef JSON_H_
#define JSON_H_

#include <Poco/JSON/Object.h>
#include <server/common/Array.h>
#include <server/common/string.h>

namespace sail {
namespace json {

// If serializeField, deserializeField are already defined for type T,
// use this templates to build a serialize function.
//
//  SEE THE COMMENT IN THE HEADER OF THIS FILE!!! VERY IMPORTANT
//
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

// string
void serializeField(Poco::JSON::Object::Ptr obj, const std::string &fieldName, const std::string &value);
bool deserializeField(Poco::Dynamic::Var obj, const std::string &fieldName, std::string *valueOut);

}
}



#endif /* JSON_H_ */
