/*
 * HtmlLog.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include "HtmlLog.h"
#include <sstream>

namespace sail {

HtmlNode::Ptr HtmlPage::make(std::string baseDir, std::string prefix) {
  return HtmlNode::Ptr(new HtmlPage(baseDir, prefix));
}

HtmlNode::Ptr HtmlPage::makeNewRoot() {
  std::stringstream ss;
  ss << _prefix << "_" << _subPageCounter;
  _subPageCounter++;
  return HtmlPage::make(_baseDir, ss.str());
}

std::string HtmlPage::localAddress() const {
  return _prefix + ".html";
}

HtmlPage::HtmlPage(const std::string &baseDir, const std::string &prefix) :
  _baseDir(baseDir), _prefix(prefix), _subPageCounter(0),
  _file(baseDir + "/" + prefix + ".html") {}


AttribValue::AttribValue(double v) {
  std::stringstream ss;
  ss << v;
  _value = ss.str();
}

AttribValue::AttribValue(int v) {
  std::stringstream ss;
  ss << v;
  _value = ss.str();
}

HtmlNode::Ptr HtmlTag::make(HtmlNode::Ptr parent, const std::string &tagName,
      const std::vector<std::pair<std::string, AttribValue> > &attribs) {
  return HtmlNode::Ptr(new HtmlTag(parent, tagName, attribs));
}

HtmlTag::HtmlTag(HtmlNode::Ptr parent, const std::string &tagName,
      const std::vector<std::pair<std::string, AttribValue> > &attribs) :
        _parent(parent), _tagName(tagName), _attributes(attribs), _stream(parent->stream()) {
  _stream << "<" << _tagName;
  for (auto attrib: _attributes) {
    _stream << " " << attrib.first << "='" << attrib.second.get() << "'";
  }
  _stream << ">";
}

HtmlTag::~HtmlTag() {
  _stream << "</" << _tagName << ">";
}



} /* namespace sail */
