/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_COMMON_CSVPARSER_H_
#define SERVER_COMMON_CSVPARSER_H_

#include <server/common/MDArray.h>
#include <server/common/string.h>

namespace sail {

MDArray<std::string, 2> parseCsv(std::istream *s);
MDArray<std::string, 2> parseCsv(std::string filename);

};

#endif /* SERVER_COMMON_CSVPARSER_H_ */
