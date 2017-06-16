/*
 * DynamicChannelValue.cpp
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#include <server/nautical/DynamicChannelValue.h>
#include <device/anemobox/DispatcherUtils.h>
#include <server/common/ArrayBuilder.h>
#include <boost/variant.hpp>

namespace sail {

namespace {

  struct CollectDynamicValues {
   public:
    ArrayBuilder<TimedValue<DynamicChannelValue>> dst;

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      auto src = Keyword::make(sourceName);
      for (auto x: coll) {
        dst.add(TimedValue<DynamicChannelValue>(
            x.time, DynamicChannelValue::make<Code>(src, x.value)));
      }
    }
  };
}

Array<TimedValue<DynamicChannelValue>> getDynamicValues(
    const Dispatcher* d) {
  CollectDynamicValues c;
  visitDispatcherChannelsConst(d, &c);
  auto data = c.dst.get();
  std::sort(data.begin(), data.end());
  return data;
}

} /* namespace sail */
