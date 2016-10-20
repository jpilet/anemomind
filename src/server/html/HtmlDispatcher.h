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

struct ChannelSummary {
  int count = 0;
  TimeStamp from, to;

  void extend(const ChannelSummary &other) {
    count += other.count;
    from = sail::earliest(from, other.from);
    to = sail::latest(to, other.to);
  }
};

void channelSummaryToHtml(
    const ChannelSummary &info, DOM::Node *dst);
void renderChannelSummaryCountToHtml(
    const ChannelSummary &info, DOM::Node *dst);

// Make a table to inspect a dispatcher
void renderDispatcherTableOverview(
    const Dispatcher *d, DOM::Node *parent,
    std::function<void(ChannelSummary, DOM::Node *)>
            channelSummaryRenderer = &renderChannelSummaryCountToHtml);

}

#endif /* SERVER_HTML_HTMLDISPATCHER_H_ */
