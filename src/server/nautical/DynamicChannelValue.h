/*
 * DynamicChannelValue.h
 *
 *  Created on: 28 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_DYNAMICCHANNELVALUE_H_
#define SERVER_NAUTICAL_DYNAMICCHANNELVALUE_H_

#include <device/anemobox/Dispatcher.h>
#include <server/common/logging.h>
#include <server/common/metaprog.h>
#include <server/common/Keyword.h>

namespace sail {

typedef meta::List<
#define LIST_DATA_TYPE(HANDLE, CODE, SHORTNAME, TYPE, DESCRIPTION) \
  TYPE,
FOREACH_CHANNEL(LIST_DATA_TYPE)
#undef LIST_DATA_TYPE
  Velocity<double>
> ChannelTypeList;

typedef meta::MakeSet<ChannelTypeList>::type ChannelTypeSet;

template <typename L> struct MakeBoostVariant {};

template <typename ... T>
struct MakeBoostVariant<meta::List<T...>> {
  typedef boost::variant<T...> type;
};

typedef MakeBoostVariant<ChannelTypeSet>::type DynamicChannelType;

//DynamicChannelType t;

class DynamicChannelValue {
public:
  DynamicChannelValue() {}

  DynamicChannelValue(
      DataCode c,
      Keyword src,
      DynamicChannelType v)
    : _code(c), _src(src), _value(v) {}

  template <DataCode c>
  static DynamicChannelValue make(
      Keyword src,
      typename TypeForCode<c>::type t) {
    return DynamicChannelValue(c, src, DynamicChannelType(t));
  }

  template <DataCode c>
  typename TypeForCode<c>::type get() const {
    CHECK(c == code());
    return boost::get<typename TypeForCode<c>::type>(_value);
  }

  DataCode code() const {
    CHECK(!_value.empty());
    return _code;
  }

  Keyword source() const {return _src;}

  DynamicChannelType value() const {
    return _value;
  }
private:
  DataCode _code = DataCode::AWA;
  Keyword _src;
  DynamicChannelType _value;
};

class NavDataset;

Array<TimedValue<DynamicChannelValue>> getDynamicValues(
    const Dispatcher* d);

} /* namespace sail */

#endif /* SERVER_NAUTICAL_DYNAMICCHANNELVALUE_H_ */
