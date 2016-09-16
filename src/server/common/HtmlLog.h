/*
 * HtmlLog.h
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 *
 * A non-thread-safe HTML generation engine that can be used
 * to output HTML-formatted log files for debugging purposes.
 */

#ifndef SERVER_COMMON_HTMLLOG_H_
#define SERVER_COMMON_HTMLLOG_H_

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>
#include <fstream>

namespace sail {

class HtmlNode {
public:
  typedef std::shared_ptr<HtmlNode> Ptr;
  virtual ~HtmlNode() {}
private:
};

class HtmlPage {
public:
  HtmlNode::Ptr make(std::string baseDir, std::string prefix);
private:
  std::string _baseDir, _prefix;
  int _subPageCounter;
  std::ofstream _file;
};

class AttribValue {
public:
  AttribValue(const std::string &s) : _value(s) {}
  AttribValue(double v);
  AttribValue(int v);
  std::string get() const;
private:
  std::string _value;
};

class HtmlTag : public HtmlNode {
public:
private:
  HtmlNode::Ptr _parent;
  std::string _tagName;
  std::vector<std::pair<std::string, AttribValue> > _attributes;
};

} /* namespace sail */

#endif /* SERVER_COMMON_HTMLLOG_H_ */
