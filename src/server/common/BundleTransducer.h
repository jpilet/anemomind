/*
 * BundleTransducer.h
 *
 *  Created on: 12 Oct 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_BUNDLETRANSDUCER_H_
#define SERVER_COMMON_BUNDLETRANSDUCER_H_

#include <server/common/Transducer.h>
#include <server/common/ArrayBuilder.h>

namespace sail {

template <typename Split, typename Step>
struct Bundle : public Step, public Transducer<Bundle<Split, Step>> {

  typedef CleanFunctionArgTypes<Split> args;
  typedef FirstType<args> input_type;

  Bundle(const Split& split, const Step& s = Step()) : _split(split), Step(s) {}

  typedef typename Step::result_type result_type;

  result_type step(result_type y, const input_type& X) {
    if (_nextBundle.empty() || !_split(_nextBundle.last(), X)) {
      _nextBundle.add(X);
      return y;
    } else {
      auto result = Step::step(y, _nextBundle.get());
      _nextBundle = ArrayBuilder<input_type>();
      _nextBundle.add(X);
      return result;
    }
  }

  result_type flush(result_type y) {
    if (_nextBundle.empty()) {
      return y;
    } else {
      return Step::step(y, _nextBundle.get());
    }
  }

  template <typename S>
  Bundle<Split, S> apply(S s) const {
    return Bundle<Split, S>(_split, s);
  }
private:
  Split _split;
  ArrayBuilder<input_type> _nextBundle;
};

template <typename Split>
Bundle<Split, UndefinedStep> trBundle(Split split) {
  return Bundle<Split, UndefinedStep>(split);
}

}
#endif /* SERVER_COMMON_BUNDLETRANSDUCER_H_ */
