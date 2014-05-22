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
#include <server/common/CommonJson.h>

namespace sail {
namespace json {



CommonJson::Ptr serialize(const Nav &nav);
void deserialize(CommonJson::Ptr x, Nav *out);

}
} /* namespace sail */

#endif /* NAVJSON_H_ */
