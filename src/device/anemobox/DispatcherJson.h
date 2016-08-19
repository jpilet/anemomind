/*
 * DispatcherJson.h
 *
 *  Created on: 18 Aug 2016
 *      Author: jonas
 */

#ifndef DEVICE_ANEMOBOX_DISPATCHERJSON_H_
#define DEVICE_ANEMOBOX_DISPATCHERJSON_H_

#include <Poco/JSON/Object.h>
#include <device/anemobox/Dispatcher.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const Dispatcher *d);

void outputJson(const Dispatcher *d, std::ostream *dst);


}
}

#endif /* DEVICE_ANEMOBOX_DISPATCHERJSON_H_ */
