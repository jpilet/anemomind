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

namespace sail {
namespace DOM {

// https://pocoproject.org/slides/170-XML.pdf

struct Node {
  Poco::XML::AutoPtr<Poco::XML::Document> document;
  Poco::XML::AutoPtr<Poco::XML::Element> element;
};

Node makeRootNode(const std::string &name);
Node makeSubNode(Node node, const std::string &name);
void addTextNode(Node node, const std::string &text);

Node initializeHtmlPage(const std::string &title);

}
} /* namespace sail */

#endif /* SERVER_COMMON_XMLUTILS_H_ */
