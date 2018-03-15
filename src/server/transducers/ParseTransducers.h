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
#include <server/transducers/Transducer.h>

namespace sail {

/*************************************
 *
 *
 *  A transducer that breaks up an input stream into individual lines.
 *
 *
 */

struct StreamLineBreaker : public NeverDone {
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
class TokenizeStepper : public NeverDone {
public:
  TokenizeStepper(IsSeparator issep) : _isSeparator(issep) {}

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
inline GenericTransducer<TokenizeStepper<IsSeparator>>
  trTokenize(IsSeparator issep) {
  return genericTransducer(TokenizeStepper<IsSeparator>(issep));
}

}



#endif /* SERVER_COMMON_PARSETRANSDUCERS_H_ */
