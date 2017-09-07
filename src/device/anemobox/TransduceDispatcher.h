/*
 * TransduceDispatchData.h
 *
 *  Created on: 7 Sep 2017
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_TRANSDUCEDISPATCHER_H_
#define DEVICE_ANEMOBOX_TRANSDUCEDISPATCHER_H_

#include <device/anemobox/Dispatcher.h>
#include <server/common/Transducer.h>

namespace sail {

  template <typename TransducerFactory>
  struct DispatchDataTransducerVisitor {
    DispatchDataTransducerVisitor(
        Clock* clk,
        const TransducerFactory& x) : clock(clk), tf(x) {}

    Clock* clock;
    const TransducerFactory& tf;
    std::shared_ptr<DispatchData> result;

    template <DataCode code, typename T>
    void visit(const TypedDispatchData<T>* data) {
      auto r = std::make_shared<TypedDispatchDataReal<T>>(
          data->dataCode(), data->source(), clock, 0);
      typename TimedSampleCollection<T>::TimedVector dst;
      auto transducer = tf.template make<code, T>();
      transduceIntoColl(transducer, &dst,
          data->dispatcher()->values());
      r->dispatcher()->insert(dst);
      result = std::static_pointer_cast<DispatchData>(r);
    }
  };

  template <typename TransducerFactory>
  std::shared_ptr<DispatchData> transduceDispatchData(
      Clock* clk,
      const std::shared_ptr<DispatchData>& src,
      const TransducerFactory& tf) {
    DispatchDataTransducerVisitor<TransducerFactory> v(clk, tf);
    src->template visitX(&v);
    return v.result;
  }
};




#endif /* DEVICE_ANEMOBOX_TRANSDUCEDISPATCHER_H_ */
