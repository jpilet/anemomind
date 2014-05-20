/*
 *  Created on: 2014-05-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef ARRAYMAPPER_H_
#define ARRAYMAPPER_H_

#define MAKE_ARRAY_MAPPER(name, SrcType, DstType, xExpr) \
  sail::Array<DstType > name(sail::Array<SrcType > arr) { \
    return arr.map<DstType >([&](const SrcType &x) {return (xExpr);}); \
  }

#define VECTORIZE_ACCESSOR(name, SrcType, DstType) \
  MAKE_ARRAY_MAPPER(name, SrcType, DstType, x.name())

#endif /* ARRAYMAPPER_H_ */
