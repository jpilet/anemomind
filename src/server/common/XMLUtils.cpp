/*
 * XMLUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include "XMLUtils.h"
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Document.h>

namespace sail {

struct Node {
  Poco::XML::Document document;
  Poco::XML::Element element;
};

} /* namespace sail */
