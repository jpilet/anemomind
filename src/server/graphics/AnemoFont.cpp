/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "AnemoFont.h"
#include <server/common/logging.h>
#include <iostream>
#include <server/common/string.h>

namespace sail {
namespace anemofont {

Renderer::Renderer() : _left(0) {
}

void Renderer::write(char c) {
  Settings s;
  switch (c) {
   case 'a': {
     constexpr bool rounded = true;
     _left += s.straightPadding;
     if (rounded) {
       auto a0 = new Arc(true, false, s, true);
       auto a = Shift::leftAt(a0, _left);
       add(a);

       auto b0 = new Vert(s, Spand(0.5, 1.0));
       auto b = Shift::middleAt(b0, a->middle());
       add(b);

       auto c0 = new Arc(true, true, s, true);
       auto c = Shift::leftAt(c0, a->rightMost() - s.intersect);
       add(c);

       auto d0 = new Vert(s, Spand(0.5, 1));
       auto d = Shift::middleAt(d0, c->middle());
       add(d);

       auto e = new MiddleBar(Spand(a->middle(), c->middle()), s);
       add(e);
     } else {
       auto a0 = new Vert(s);
       auto a = Shift::leftAt(a0, _left);
       add(a);

       auto b0 = new Arc(true, true, s);
       auto b = Shift::leftAt(b0, a->rightMost() - s.intersect);
       add(b);

       auto c0 = new Vert(s, Spand(0.5, 1));
       auto c = Shift::middleAt(c0, b->middle());
       add(c);

       auto d = new MiddleBar(Spand(a->middle(), b->middle()), s);
       add(d);
     }
      _left += s.straightPadding;
     break;
   }
   case 'n': {
     _left += s.straightPadding;
     auto a0 = new Vert(s);
     auto a = Shift::leftAt(a0, _left);
     add(a);

     auto b0 = new Arc(true, true, s);
     auto b = Shift::leftAt(b0, a->rightMost() - s.intersect);
     add(b);

     auto c0 = new Vert(s, Spand(0.5, 1));
     auto c = Shift::middleAt(c0, b->middle());
     add(c);
     _left += s.straightPadding;
     break;
   }
   case 'e': {
     auto a0 = new Arc(true, false, s);
     auto a = Shift::leftAt(a0, _left);
     add(a);

     auto b0 = new Arc(false, false, s);
     auto b = Shift::middleAt(b0, a->middle());
     add(b);

     auto d = new MiddleBar(Spand(a->middle(), a->rightMost()), s);
     add(d);

     break;
   }
   case 'm': {
     _left += s.straightPadding;
     auto a0 = new Arc(true, false, s, true);
     auto a = Shift::leftAt(a0, _left);
     add(a);

     auto g0 = new Vert(s, Spand(0.5, 1.0));
     auto g = Shift::middleAt(g0, a->middle());
     add(g);

     auto b0 = new Arc(true, true, s, true);
     auto b = Shift::leftAt(b0, a->rightMost() - s.intersect);
     add(b);

     auto c0 = new Vert(s, Spand(0.5, 1));
     auto c = Shift::middleAt(c0, b->middle());
     add(c);

     auto d0 = new Arc(true, false, s, true);
     auto d = Shift::middleAt(d0, c->middle());
     add(d);

     auto e0 = new Arc(true, true, s, true);
     auto e = Shift::leftAt(e0, d->rightMost() - s.intersect);
     add(e);

     auto f0 = new Vert(s, Spand(0.5, 1));
     auto f = Shift::middleAt(f0, e->middle());
     add(f);
     _left += s.straightPadding;



     break;
   }
   case 'o': {
     auto a0 = new Arc(true, false, s, true);
     auto a = Shift::leftAt(a0, _left);
     add(a);

     auto b0 = new Arc(false, false, s, true);
     auto b = Shift::middleAt(b0, a->middle());
     add(b);

     auto c0 = new Arc(true, true, s, true);
     auto c = Shift::leftAt(c0, b->rightMost() - s.intersect);
     add(c);

     auto d0 = new Arc(false, true, s, true);
     auto d = Shift::middleAt(d0, c->middle());
     add(d);

     break;
   }
   case 'i': {
     _left += s.straightPadding;
     auto a0 = new Vert(s);
     auto a = Shift::leftAt(a0, _left);
     add(a);
     _left += s.straightPadding;
     break;
   }
   case 'd': {
     _left += s.straightPadding;
     auto a0 = new Vert(s);
     auto a = Shift::leftAt(a0, _left);
     add(a);

     auto b0 = new Arc(true, true, s);
     auto b = Shift::leftAt(b0, a->rightMost() - s.intersect);
     add(b);

     auto c0 = new Arc(false, true, s);
     auto c = Shift::middleAt(c0, b->middle());
     add(c);
     break;
   }
   default:
     LOG(FATAL) << "This character is not implemented";
   };

   _left += s.letterSpacing;
}

Renderer::Vec Renderer::operator() (double x, double y) const {
  int count = _primitives.size();
  Renderer::Vec fg{1, 0, 1};
  Renderer::Vec bg{1, 1, 1};
  for (int i = 0; i < count; i++) {
    if (_primitives[i]->inside(x, y)) {
      return fg;
    }
  }
  return bg;
}


}
} /* namespace mmm */
