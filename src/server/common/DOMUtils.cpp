/*
 * XMLUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include <server/common/DOMUtils.h>
#include <Poco/DOM/Text.h>

namespace sail {
namespace DOM {

using namespace Poco;
using namespace Poco::XML;

Node makeRootNode(const std::string &name) {
  Node dst;
  dst.document = new Poco::XML::Document();
  dst.element = dst.document->createElement(name);
  dst.document->appendChild(dst.element);
  return dst;
}

Node makeSubNode(Node node, const std::string &name) {
  Node dst;
  dst.document = node.document;
  dst.element = node.document->createElement(name);
  node.element->appendChild(dst.element);
  return dst;
}

void addTextNode(Node node, const std::string &text) {
  auto x = node.document->createTextNode(text);
  node.element->appendChild(x);
}

Node initializeHtmlPage(const std::string &titleString) {
  auto page = makeRootNode("html");
  auto head = makeSubNode(page, "head");
  auto title = makeSubNode(head, "title");
  addTextNode(title, titleString);
  auto body = makeSubNode(page, "body");
  return body;
}


}
}
