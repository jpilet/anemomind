/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "AnemoFont.h"
#include <server/common/logging.h>

namespace sail {
namespace anemofont {

Renderer::Renderer() : _left(0) {
}

void Renderer::write(char c) {
  switch (c) {
   case 'a': {break;}
   case 'n': {break;}
   case 'e': {break;}
   case 'm': {break;}
   case 'o': {break;}
   case 'i': {break;}
   case 'd': {break;}
   default:
     LOG(FATAL) << "This character is not implemented";
   };
}


}
} /* namespace mmm */
