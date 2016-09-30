/*
 * HtmlLog.cpp
 *
 *  Created on: 16 Sep 2016
 *      Author: jonas
 */

#include "HtmlLog.h"
#include <sstream>
#include <server/common/logging.h>

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
  if (!bool(parent)) {
    return HtmlNode::Ptr();
  }
  return HtmlNode::Ptr(new HtmlTag(parent, tagName, attribs));
}

HtmlNode::Ptr HtmlTag::initializePage(
    HtmlNode::Ptr emptyPage,
    const std::string &titleStr) {
  {
    auto head = HtmlTag::make(emptyPage, "head");
    {
      auto title = HtmlTag::make(head, "title");
      title->stream() << titleStr;
    }{
      auto style = HtmlTag::make(head, "style");
      style->stream() << "td, th {border: 1px solid black;} .warning {color: orange} .error {color: red} .success {color: green}";
    }
  }
  return HtmlTag::make(emptyPage, "body");
}

void HtmlTag::tagWithData(HtmlNode::Ptr parent,
      const std::string &tagName,
      const std::string &data) {
  tagWithData(parent, tagName, {}, data);
}

void HtmlTag::tagWithData(HtmlNode::Ptr parent,
    const std::string &tagName,
    const std::vector<std::pair<std::string, AttribValue> > &attribs,
    const std::string &data) {
  if (parent) {
    auto dst = make(parent, tagName, attribs);
    dst->stream() << data;
  }
}

HtmlNode::Ptr HtmlTag::linkToSubPage(
    HtmlNode::Ptr parent,
    const std::string &linkText) {
  if (parent) {
    auto subPage = parent->makeNewRoot();
    auto link = HtmlTag::make(parent, "a",
        {{"href", subPage->localAddress()}});
    link->stream() << linkText;
    return subPage;
  }
  return HtmlNode::Ptr();
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

void renderTable(
    HtmlNode::Ptr parent,
    int rows, int cols,
    std::function<bool(int,int)> isHeader,
    std::function<void(HtmlNode::Ptr, int, int)> renderCell) {
  if (parent) {
    auto table = HtmlTag::make(parent, "table");
    for (int i = 0; i < rows; i++) {
      auto row = HtmlTag::make(table, "tr");
      for (int j = 0; j < cols; j++) {
        auto element = HtmlTag::make(row, isHeader(i, j)? "th" : "td");
        renderCell(element, i, j);
      }
    }
  }
}

SubTable vcat(const SubTable &a, const SubTable &b) {
  CHECK(a.cols == b.cols);
  CHECK(a.renderer);
  CHECK(b.renderer);
  return SubTable{
    a.rows + b.rows,
    a.cols,
    [=](HtmlNode::Ptr dst, int i, int j) {
      if (i < a.rows) {
        a.renderer(dst, i, j);
      } else {
        b.renderer(dst, i - a.rows, j);
      }
    }
  };
}

SubTable hcat(const SubTable &a, const SubTable &b) {
  CHECK(a.rows == b.rows);
  CHECK(a.renderer);
  CHECK(b.renderer);
  return SubTable{
    a.rows,
    a.cols + b.cols,
    [=](HtmlNode::Ptr dst, int i, int j) {
      if (j < a.cols) {
        a.renderer(dst, i, j);
      } else {
        b.renderer(dst, i, j - a.cols);
      }
    }
  };
}

SubTable SubTable::cell(const std::string &cellType,
    int r, int c, Renderer ren) {
  return SubTable{r, c,
      [=](HtmlNode::Ptr dst, int i, int j) {
    auto cell = HtmlTag::make(dst, cellType);
    ren(cell, i, j);
  }};
}

SubTable SubTable::cell(
    int r, int c, Renderer ren) {
  return cell("td", r, c, ren);
}

void renderTable(HtmlNode::Ptr dst, const SubTable &src) {
  auto table = HtmlTag::make(dst, "table");
  for (int i = 0; i < src.rows; i++) {
    auto row = HtmlTag::make(table, "tr");
    for (int j = 0; j < src.cols; j++) {
      src.renderer(row, i, j);
    }
  }
}


SubTable::SubTable(int r, int c, Renderer ren) : rows(r), cols(c),
    renderer(ren) {}

SubTable SubTable::header(
    int r, int c, Renderer ren) {
  return cell("th", r, c, ren);
}



} /* namespace sail */
