/*
 *  Created on: 2014-05-23
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FLOWFIELDJSON_H_
#define FLOWFIELDJSON_H_

#include <server/common/CommonJson.h>
#include <server/nautical/FlowField.h>

namespace sail {
namespace json {

CommonJson::Ptr serialize(const FlowField &x);
void deserialize(CommonJson::Ptr obj, FlowField *dst);

}
} /* namespace sail */

#endif /* FLOWFIELDJSON_H_ */
