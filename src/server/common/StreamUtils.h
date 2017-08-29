/*
 * RedirectOstream.h
 *
 *  Created on: 21 Apr 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_STREAMUTILS_H_
#define SERVER_COMMON_STREAMUTILS_H_

#include <boost/noncopyable.hpp>
#include <iosfwd>

namespace sail {

// Directing to null might work:
// http://stackoverflow.com/questions/14845446/how-redirect-stdostream-to-a-file-or-dev-null-in-c-and-linux

// Please be careful using this in multithreaded code with global
// variables such as std::cout and std::cerr.
class RedirectOstream : public boost::noncopyable {
public:
  RedirectOstream(std::ostream* target, std::streambuf* dst);
  ~RedirectOstream();
private:
  std::ostream* _target;
  std::streambuf* _backup;
};

} /* namespace sail */

#endif /* SERVER_COMMON_STREAMUTILS_H_ */
