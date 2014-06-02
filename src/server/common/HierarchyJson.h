/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HIERARCHYJSON_H_
#define HIERARCHYJSON_H_

#include <server/common/Hierarchy.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

namespace sail {
namespace json {


// HNode
Poco::Dynamic::Var serialize(const HNode &x);
void deserialize(Poco::Dynamic::Var src, HNode *dst);

//Poco::JSON::Array serialize(Array<HNode> src);
//void deserialize(Poco::JSON::Array src, Array<HNode> *dst);


// HTree
Poco::Dynamic::Var serialize(std::shared_ptr<HTree> &x);
void deserialize(Poco::Dynamic::Var src, std::shared_ptr<HTree> *dst);

//Poco::JSON::Array serialize(Array<std::shared_ptr<HTree> > x);
//void deserialize(Poco::JSON::Array src, Array<std::shared_ptr<HTree> > *dst);


}
} /* namespace sail */

#endif /* HIERARCHYJSON_H_ */
