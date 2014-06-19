/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef TESTDOMAINJSON_H_
#define TESTDOMAINJSON_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
class TestSpaceDomain;
class TestTimeDomain;
class TestDomain;
namespace json {

Poco::Dynamic::Var serialize(const TestSpaceDomain &src);
bool deserialize(Poco::Dynamic::Var src, TestSpaceDomain *dst);

Poco::Dynamic::Var serialize(const TestTimeDomain &src);
bool deserialize(Poco::Dynamic::Var src, TestTimeDomain *dst);

Poco::Dynamic::Var serialize(const TestDomain &src);
bool deserialize(Poco::Dynamic::Var src, TestDomain *dst);

}
} /* namespace sail */

#endif /* TESTDOMAINJSON_H_ */
