/*
 *  Created on: 2014-06-19
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef USERHINTJSON_H_
#define USERHINTJSON_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
class UserHint;
namespace json {

Poco::Dynamic::Var serialize(const UserHint &x);
bool deserialize(Poco::Dynamic::Var src, UserHint *dst);

}
} /* namespace sail */

#endif /* USERHINTJSON_H_ */
