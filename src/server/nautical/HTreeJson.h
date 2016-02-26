/*
 *  Created on: 2014-10-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HTREEJSON_H_
#define HTREEJSON_H_

#include <server/common/Array.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <server/nautical/NavCompatibility.h>
#include <server/common/Hierarchy.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serializeMapped(std::shared_ptr<HTree> x, NavDataset navs, Array<HNode> nodeInfo);
Poco::Dynamic::Var serializeMapped(Array<std::shared_ptr<HTree> > x, NavDataset navs, Array<HNode> nodeInfo);

}
} /* namespace sail */

#endif /* HTREEJSON_H_ */
