/*
 * XMLUtils.h
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_XMLUTILS_H_
#define SERVER_COMMON_XMLUTILS_H_

#include <Poco/DOM/Element.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/Path.h>
#include <server/common/Array.h>
#include <memory>
#include <sstream>

namespace sail {
namespace DOM {

// https://pocoproject.org/slides/170-XML.pdf

// Writes a document to file once the node goes out of scope.
// This is useful when automatically generating a tree of web
// pages, and we don't want to keep track of saving every page.
class PageWriter {
public:
  typedef std::shared_ptr<PageWriter> Ptr;

  PageWriter(
      const std::string &basePath,
      const std::string &name,
      const Poco::XML::AutoPtr<Poco::XML::Document> &doc);
  std::string localFilename() const {return _name + ".html";}
  std::string fullFilename() const;
  PageWriter::Ptr makeSubPageWriter(
      Poco::XML::AutoPtr<Poco::XML::Document> doc);
  ~PageWriter();

  std::string generateName();
  Poco::Path generatePath(const std::string &suffix);
private:
  PageWriter(const PageWriter &other) = delete;
  PageWriter &operator=(const PageWriter &other) = delete;

  int _counter = 0;
  std::string _basePath;
  std::string _name;
  Poco::XML::AutoPtr<Poco::XML::Document> _document;
};

struct Node {
  Poco::XML::AutoPtr<Poco::XML::Document> document;
  Poco::XML::AutoPtr<Poco::XML::Element> element;
  PageWriter::Ptr writer;

  bool defined() const;
  void setAttribute(
      const std::string &key,
      const std::string &value);

  void setClass(const std::string &s);
  void success();
  void warning();
  void error();
  void interesting();
};

Poco::Path generatePath(const Node& src, const std::string &suffix);

// Even if functions below accept a pointer to a Node,
// that pointer must never be a null pointer. To represent
// the absence of a Node, or an undefined Node, use a default
// constructed one.
//
// None of the functions below will crash if we pass
// a default-constructed Node, they will just do the minimum
// work possible.
Node makeRootNode(const std::string &name);
Node makeSubNode(Node *parent, const std::string &name);
void addTextNode(Node *parent, const std::string &text);
void addLine(Node *parent, const std::string &text);
Node addSubTextNode(Node *node,
    const std::string &name,
    const std::string &data);

Node makeBasicHtmlPage(const std::string &title,
    const std::string &basePath,
    const std::string &name);
Node makeBasicHtmlPage(const std::string &titleString);

void writeHtmlFile(
    const std::string &filename,
    Poco::XML::AutoPtr<Poco::XML::Document> document);

Node linkToSubPage(Node *parent, const std::string title);
Poco::Path makeGeneratedImageNode(
    Node *node, const std::string &filenameSuffix);
template <typename T> std::string objectToString(const T &x) {
  std::stringstream ss; ss << x; return ss.str();
}
Node displayLabeledString(const char *label, const std::string &s, Node *dst);
#define HTML_DISPLAY(x, dst) if ((dst)->defined()) {DOM::displayLabeledString(#x, objectToString(x), (dst));}

}
} /* namespace sail */

#endif /* SERVER_COMMON_XMLUTILS_H_ */
