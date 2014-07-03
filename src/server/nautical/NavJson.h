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

Poco::Dynamic::Var serialize(const Nav &nav);
bool deserialize(Poco::Dynamic::Var x, Nav *out);

}  // namespace json
}  // namespace sail

#endif /* NAVJSON_H_ */
