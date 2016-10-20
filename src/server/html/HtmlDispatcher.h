/*
 * HtmlDispatcher.h
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_HTML_HTMLDISPATCHER_H_
#define SERVER_HTML_HTMLDISPATCHER_H_

#include <device/anemobox/Dispatcher.h>
#include <server/common/DOMUtils.h>

namespace sail {

// Make a table to inspect a dispatcher
void renderDispatcherTableOverview(
    const Dispatcher *d, const DOM::Node &parent);

}

#endif /* SERVER_HTML_HTMLDISPATCHER_H_ */
