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

namespace sail {
namespace DOM {

struct Node {
  Poco::XML::Document document;
  Poco::XML::Element element;
};


}
} /* namespace sail */

#endif /* SERVER_COMMON_XMLUTILS_H_ */
