/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Provides information
 */

#ifndef TESTENV_H_
#define TESTENV_H_

#include <Poco/Path.h>

namespace sail {

class TestEnv {
 public:
  TestEnv();
  Poco::Path root() const {return _root;}
  Poco::Path datasets() const {return _datasets;}
 private:
  Poco::Path _root;
  Poco::Path _datasets;
};

} /* namespace sail */

#endif /* TESTENV_H_ */
