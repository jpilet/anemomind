/*
 * XMLUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include <server/common/DOMUtils.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/XML/XMLWriter.h>
#include <fstream>
#include <server/common/PathBuilder.h>
#include <sstream>

namespace sail {
namespace DOM {

using namespace Poco;
using namespace Poco::XML;


PageWriter::PageWriter(
    const std::string &basePath,
    const std::string &localFilename,
    const Poco::XML::AutoPtr<Poco::XML::Document> &doc) :
      _basePath(basePath),
      _name(localFilename),
      _document(doc),
      _counter(0) {}

std::string PageWriter::fullFilename() const {
  return PathBuilder::makeDirectory(_basePath)
    .makeFile(localFilename()).get().toString();
}

PageWriter::Ptr PageWriter::makeSubPageWriter(AutoPtr<Document> doc) {
  std::stringstream ss;
  ss << _name << "_" << _counter;
  _counter++;
  return std::make_shared<PageWriter>(_basePath, ss.str(), doc);
}

PageWriter::~PageWriter() {
  std::ofstream file(fullFilename());
  file << "<!DOCTYPE html>\n";
  DOMWriter writer;
  writer.setNewLine("\n");
  writer.writeNode(file, _document);
}

Node makeRootNode(const std::string &name) {
  Node dst;
  dst.document = new Poco::XML::Document();
  dst.element = dst.document->createElement(name);
  dst.document->appendChild(dst.element);
  return dst;
}

Node makeSubNode(Node node, const std::string &name) {
  Node dst = node;
  dst.element = node.document->createElement(name);
  node.element->appendChild(dst.element);
  return dst;
}

void addTextNode(Node node, const std::string &text) {
  auto x = node.document->createTextNode(text);
  node.element->appendChild(x);
}

Node makeBasicHtmlPage(const std::string &titleString,
    const std::string &basePath,
    const std::string &name) {
  auto page = makeRootNode("html");
  page.writer = std::make_shared<PageWriter>(
      basePath, name, page.document);
  auto head = makeSubNode(page, "head");
  auto title = makeSubNode(head, "title");
  addTextNode(title, titleString);
  auto body = makeSubNode(page, "body");
  return body;
}


void renderPage(const std::string &filename, const Node &node) {
}


}
}
