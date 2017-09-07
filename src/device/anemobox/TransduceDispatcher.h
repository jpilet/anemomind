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

  template <typename Transducer>
  struct DispatchDataTransducerVisitor {
    DispatchDataTransducerVisitor(
        Clock* clk,
        const Transducer& x) : clock(clk), transducer(x) {}

    Clock* clock;
    const Transducer& transducer;
    std::shared_ptr<DispatchData> result;

    template <DataCode code, typename T>
    void visit(const TypedDispatchData<T>* data) {
      auto r = std::make_shared<TypedDispatchDataReal<T>>(
          data->dataCode(), data->source(), clock, 0);
      typename TimedSampleCollection<T>::TimedVector dst;
      std::cout << "Number of input samples: " << data->dispatcher()->values().size() << std::endl;
      std::cout << "Number of output samples: " << dst.size() << std::endl;
      transduceIntoColl(transducer, &dst,
          data->dispatcher()->values());

      r->dispatcher()->insert(dst);
      result = std::static_pointer_cast<DispatchData>(r);
    }
  };

  template <typename Transducer>
  std::shared_ptr<DispatchData> transduceDispatchData(
      Clock* clk,
      const std::shared_ptr<DispatchData>& src,
      const Transducer& transducer) {
    DispatchDataTransducerVisitor<Transducer> v(clk, transducer);
    src->template visitX(&v);
    CHECK(bool(v.result));
    return v.result;
  }
};




#endif /* DEVICE_ANEMOBOX_TRANSDUCEDISPATCHER_H_ */
