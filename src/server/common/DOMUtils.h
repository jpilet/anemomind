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
};

Node makeRootNode(const std::string &name);
Node makeSubNode(Node *parent, const std::string &name);
void addTextNode(Node *parent, const std::string &text);
void addSubTextNode(Node *node,
    const std::string &name,
    const std::string &data);

Node makeBasicHtmlPage(const std::string &title,
    const std::string &basePath,
    const std::string &name);
Node makeBasicHtmlPage(const std::string &titleString);

void writeHtmlFile(
    const std::string &filename,
    Poco::XML::AutoPtr<Poco::XML::Document> document);

Node linkToSubPage(Node parent, const std::string title);
Poco::Path makeGeneratedImageNode(
    Node node, const std::string &filenameSuffix);

// Just a helper class for generating basic tables,
// without fancy features such as cells spanning multiple
// rows or columns.
class BasicTable {
public:

  // This function is used to populate a parent node.
  // By providing the parent node (that will be of type <tr>...</tr>),
  // the function can decide whether the cell should be a

  typedef std::function<
      void(Node parent, int i, int j)> CellFunction;

  BasicTable(
      int rows, int cols,
      CellFunction cellFunction);

  BasicTable(int rows, int cols,
      bool isHeader, std::function<std::string(int, int)> f);

  BasicTable vcat(const BasicTable &other) const;
  BasicTable hcat(const BasicTable &other) const;

  void attachTo(Node parent) const;

  int rows() const {return _rows;}
  int cols() const {return _cols;}
  CellFunction cellFunction() const {
    return _cellFunction;
  }

  static BasicTable row(bool isHeader,
      const Array<std::string> &iterms);
  static BasicTable col(bool isHeader,
      const Array<std::string> &items);
private:
  int _rows, _cols;
  CellFunction _cellFunction;
};

}
} /* namespace sail */

#endif /* SERVER_COMMON_XMLUTILS_H_ */
