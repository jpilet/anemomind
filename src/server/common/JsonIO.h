/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef JSONIO_H_
#define JSONIO_H_

#include <Poco/Dynamic/Var.h>

namespace sail {
namespace json {

Poco::Dynamic::Var load(const std::string &filename);
void save(const std::string &filename, Poco::Dynamic::Var x);

}
}

#endif /* JSONIO_H_ */
