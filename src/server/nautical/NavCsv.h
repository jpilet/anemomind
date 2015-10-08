/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_NAVCSV_H_
#define SERVER_NAUTICAL_NAVCSV_H_

#include <server/nautical/Nav.h>

namespace sail {
namespace NavCsv {

Array<Nav> parse(MDArray<std::string, 2> table);
Array<Nav> parse(std::string filename);
Array<Nav> parse(std::istream *s);

}
}

#endif /* SERVER_NAUTICAL_NAVCSV_H_ */
