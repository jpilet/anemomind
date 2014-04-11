/*
 *  Created on: 2014-10-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef HTREEJSON_H_
#define HTREEJSON_H_

#include <server/common/Array.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <server/nautical/Nav.h>
#include <server/common/Hierarchy.h>

namespace sail {
namespace json {

Poco::JSON::Object::Ptr serializeMapped(std::shared_ptr<HTree> x, Array<Nav> navs, Array<HNode> nodeInfo);
Poco::JSON::Array serializeMapped(Array<std::shared_ptr<HTree> > x, Array<Nav> navs, Array<HNode> nodeInfo);

}
} /* namespace sail */

#endif /* HTREEJSON_H_ */
