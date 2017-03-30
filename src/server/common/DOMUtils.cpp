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
#include <iostream>
#include <server/common/logging.h>

namespace sail {
namespace DOM {

using namespace Poco;
using namespace Poco::XML;


PageWriter::PageWriter(
    const std::string &basePath,
    const std::string &name,
    const Poco::XML::AutoPtr<Poco::XML::Document> &doc) :
      _basePath(basePath),
      _name(name),
      _document(doc),
      _counter(0) {}

std::string PageWriter::fullFilename() const {
  return PathBuilder::makeDirectory(_basePath)
    .makeFile(localFilename()).get().toString();
}

std::string PageWriter::generateName() {
  std::stringstream ss;
  ss << _name << "_" << _counter;
  _counter++;
  return ss.str();
}

PageWriter::Ptr PageWriter::makeSubPageWriter(AutoPtr<Document> doc) {
  return std::make_shared<PageWriter>(_basePath, generateName(), doc);
}

void writeHtmlFile(
    const std::string &filename,
    Poco::XML::AutoPtr<Poco::XML::Document> document) {
  std::ofstream file(filename);
  file << "<!DOCTYPE html>\n";
  DOMWriter writer;
  writer.setNewLine("\n");
  writer.writeNode(file, document);
}


PageWriter::~PageWriter() {
  std::cout << "Output HTML to " << fullFilename() << std::endl;
  writeHtmlFile(fullFilename(), _document);
}

Poco::Path PageWriter::generatePath(const std::string &suffix) {
  return PathBuilder::makeDirectory(_basePath)
    .makeFile(generateName() + suffix).get();
}

bool Node::defined() const {
  return bool(element);
}

void Node::setAttribute(
    const std::string &key,
    const std::string &value) {
  if (defined()) {
    element->setAttribute(key, value);
  }
}

void Node::setClass(const std::string &s) {
  setAttribute("class", s);
}

void Node::success() {
  setClass("success");
}
void Node::warning() {
  setClass("warning");
}
void Node::error() {
  setClass("error");
}

Node makeRootNode(const std::string &name) {
  Node dst;
  dst.document = new Poco::XML::Document();
  dst.element = dst.document->createElement(name);
  dst.document->appendChild(dst.element);
  return dst;
}

Node makeSubNode(Node *node, const std::string &name) {
  CHECK(node != nullptr);
  if (!node->defined()) {
    return Node();
  }
  Node dst = *node;
  dst.element = node->document->createElement(name);
  node->element->appendChild(dst.element);
  return dst;
}

Node addSubTextNode(Node *node, const std::string &name,
    const std::string &data) {
  CHECK(node != nullptr);
  if (!node->defined()) {
    return Node();
  }
  auto x = makeSubNode(node, name);
  addTextNode(&x, data);
  return x;
}

void addTextNode(Node *node, const std::string &text) {
  CHECK(node != nullptr);
  if (!node->defined()) {
    return;
  }
  auto x = node->document->createTextNode(text);
  node->element->appendChild(x);
}

void addLine(Node *parent, const std::string &text) {
  addTextNode(parent, text + "\n");
}

Node makeBasicHtmlPage(const std::string &titleString) {
  auto page = makeRootNode("html");
  auto head = makeSubNode(&page, "head");
  auto title = makeSubNode(&head, "title");
  addSubTextNode(&head, "style",
      "body {padding: 2em; font-family: Ubuntu Mono;} td, th {border: 1px solid black;} svg {margin: 30px; border: 1px solid black;} .warning {color: orange} .error {color: red} .success {color: green}");
  addTextNode(&title, titleString);
  auto body = makeSubNode(&page, "body");
  return body;
}

Node makeBasicHtmlPage(const std::string &titleString,
    const std::string &basePath,
    const std::string &name) {
  auto page = makeBasicHtmlPage(titleString);
  page.writer = std::make_shared<PageWriter>(
    basePath, name, page.document);
  return page;
}


Node linkToSubPage(Node *parent, const std::string title) {
  CHECK(parent != nullptr);
  if (!parent->defined()) {
    return Node();
  }
  auto subPage = makeBasicHtmlPage(title);
  subPage.writer = parent->writer->makeSubPageWriter(subPage.document);
  auto a = makeSubNode(parent, "a");
  a.element->setAttribute(toXMLString("href"),
      toXMLString(subPage.writer->localFilename()));
  addTextNode(&a, title);
  return subPage;
}

Poco::Path makeGeneratedImageNode(Node *node,
    const std::string &filenameSuffix) {
  CHECK(node != nullptr);
  if (!node->defined()) {
    return Poco::Path();
  }
  Poco::Path p = node->writer->generatePath(filenameSuffix);
  auto img = DOM::makeSubNode(node, "img");
  img.element->setAttribute(
      Poco::XML::toXMLString("src"),
      Poco::XML::toXMLString(p.getFileName()));
  return p;
}

}
}
