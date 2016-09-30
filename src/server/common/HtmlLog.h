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
#include <server/common/MDArray.h>

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
  AttribValue(const char *s) : _value(s) {}
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
  static void tagWithData(HtmlNode::Ptr parent,
      const std::string &tagName,
      const std::vector<std::pair<std::string, AttribValue> > &attribs,
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

void renderTable(
    HtmlNode::Ptr parent,
    int rows, int cols,
    std::function<bool(int,int)> isHeader,
    std::function<void(HtmlNode::Ptr, int, int)> renderCell);

struct SubTable {
  typedef std::function<void(HtmlNode::Ptr, int, int)> Renderer;
  int rows = 0;
  int cols = 0;
  Renderer renderer;

  SubTable(int r, int c, Renderer ren);

  static SubTable header(int r, int c, Renderer ren);
  static SubTable cell(int r, int c, Renderer ren);
  static SubTable cell(
      const std::string &cellType, int r, int c,
      Renderer ren);
};

SubTable vcat(const SubTable &a, const SubTable &b);
SubTable hcat(const SubTable &a, const SubTable &b);

void renderTable(HtmlNode::Ptr dst, const SubTable &table);



} /* namespace sail */

#endif /* SERVER_COMMON_HTMLLOG_H_ */
