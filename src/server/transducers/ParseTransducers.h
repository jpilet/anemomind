/*
 * StreamTransducerUtils.h
 *
 *  Created on: 9 Mar 2018
 *      Author: jonas
 *
 *  Transducers for text parsing.
 */

#ifndef SERVER_COMMON_PARSETRANSDUCERS_H_
#define SERVER_COMMON_PARSETRANSDUCERS_H_

#include <string>
#include <istream>
#include <server/common/Transducer.h>

namespace sail {

/*************************************
 *
 *
 *  A transducer that breaks up an input stream into individual lines.
 *
 *
 */

struct StreamLineBreaker : public NeverReduced {
  template <typename R>
  void apply(R* result, const std::shared_ptr<std::istream>& src) {
    if (!src) {
      return;
    }
    std::string line;
    while (std::getline(*src, line)) {
      result->add(line);
    }
  }

  template <typename R>
  void flush(R* result) {
    result->flush();
  }
};

inline GenericTransducer<StreamLineBreaker> trStreamLines() {
  return genericTransducer(StreamLineBreaker());
}

template <typename IsSeparator>
class StringSplitStepper : public NeverReduced {
public:
  StringSplitStepper(IsSeparator issep) : _isSeparator(issep) {}

  template <typename R>
  void apply(R* result, char c) {
    if (_isSeparator(c)) {
      flush(result);
    } else {
      _s += c;
    }
  }

  template <typename R>
  void flush(R* result) {
    if (!_s.empty()) {
      result->add(_s);
      _s = "";
    }
  }
private:
  IsSeparator _isSeparator;
  std::string _s;
};

template <typename IsSeparator>
inline GenericTransducer<StringSplitStepper<IsSeparator>>
  trSplitString(IsSeparator issep) {
  return genericTransducer(StringSplitStepper<IsSeparator>(issep));
}

}



#endif /* SERVER_COMMON_PARSETRANSDUCERS_H_ */
