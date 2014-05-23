/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef LINEKMJSON_H_
#define LINEKMJSON_H_

#include <server/common/CommonJson.h>

namespace sail {
class LineKM;

namespace json {
CommonJson::Ptr serialize(const LineKM &x);
void deserialize(CommonJson::Ptr src, LineKM *dst);

}
} /* namespace sail */

#endif /* LINEKMJSON_H_ */
