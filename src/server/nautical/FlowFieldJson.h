/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FLOWFIELDJSON_H_
#define FLOWFIELDJSON_H_

#include <Poco/Dynamic/Var.h>

namespace sail {

class FlowField;
namespace json {

Poco::Dynamic::Var serialize(const FlowField &x);
bool deserialize(Poco::Dynamic::Var src, FlowField *dst);

}
} /* namespace sail */

#endif /* FLOWFIELDJSON_H_ */
