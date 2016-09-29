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
  virtual std::ostream &stream() = 0;
  virtual HtmlNode::Ptr makeNewRoot() = 0;
  virtual std::string localAddress() const = 0;
private:
};

class HtmlPage : public HtmlNode {
public:
  static HtmlNode::Ptr make(std::string baseDir, std::string prefix);
  std::ostream &stream() override {return _file;}
  HtmlNode::Ptr makeNewRoot() override;
  std::string localAddress() const override;
private:
  HtmlPage(const std::string &baseDir, const std::string &prefix);
  std::string _baseDir, _prefix;
  int _subPageCounter;
  std::ofstream _file;
};

class AttribValue {
public:
  AttribValue(const std::string &s) : _value(s) {}
  AttribValue(double v);
  AttribValue(int v);
  std::string get() const {return _value;}
private:
  std::string _value;
};

class HtmlTag : public HtmlNode {
public:
  static HtmlNode::Ptr make(HtmlNode::Ptr parent, const std::string &tagName,
        const std::vector<std::pair<std::string, AttribValue> > &attribs
           = std::vector<std::pair<std::string, AttribValue> >());

  static HtmlNode::Ptr initializePage(
      HtmlNode::Ptr emptyPage,
      const std::string &title);

  static void tagWithData(HtmlNode::Ptr parent,
      const std::string &tagName,
      const std::string &data);

  static HtmlNode::Ptr linkToSubPage(
      HtmlNode::Ptr parent,
      const std::string &linkText);


  std::ostream &stream() override {return _stream;}

  HtmlNode::Ptr makeNewRoot() override {return _parent->makeNewRoot();}
  std::string localAddress() const override {return _parent->localAddress();}


  virtual ~HtmlTag();
private:
  HtmlTag(HtmlNode::Ptr parent, const std::string &tagName,
        const std::vector<std::pair<std::string, AttribValue> > &attribs);


  HtmlNode::Ptr _parent;
  std::string _tagName;
  std::vector<std::pair<std::string, AttribValue> > _attributes;
  std::ostream &_stream;
};

} /* namespace sail */

#endif /* SERVER_COMMON_HTMLLOG_H_ */
