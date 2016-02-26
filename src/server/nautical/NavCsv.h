/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_NAVCSV_H_
#define SERVER_NAUTICAL_NAVCSV_H_

#include <server/nautical/NavCompatibility.h>

namespace sail {
namespace NavCsv {

NavDataset parse(MDArray<std::string, 2> table);
NavDataset parse(std::string filename);
NavDataset parse(std::istream *s);

}
}

#endif /* SERVER_NAUTICAL_NAVCSV_H_ */
