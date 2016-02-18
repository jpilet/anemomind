/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_NAVCSV_H_
#define SERVER_NAUTICAL_NAVCSV_H_

#include <server/nautical/Nav.h>

namespace sail {
namespace NavCsv {

NavCollection parse(MDArray<std::string, 2> table);
NavCollection parse(std::string filename);
NavCollection parse(std::istream *s);

}
}

#endif /* SERVER_NAUTICAL_NAVCSV_H_ */
