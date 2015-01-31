/*
 *  Created on: May 22, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef LINEKMJSON_H_
#define LINEKMJSON_H_

#include <server/common/LineKM.h>
#include <Poco/Dynamic/Var.h>

namespace sail {
//class LineKM;

namespace json {

Poco::Dynamic::Var serialize(const LineKM &x);
bool deserialize(Poco::Dynamic::Var src, LineKM *dst);

}
} /* namespace sail */

#endif /* LINEKMJSON_H_ */
