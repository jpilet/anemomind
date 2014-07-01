/*
 *  Created on: Jul 1, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef VERSIONEDENUM_H_
#define VERSIONEDENUM_H_

namespace sail {

unsigned int calcChecksum(const char *data);

/*
 * To be declared in a class.
 */
#define VERSIONED_ENUM(ENUMNAME, ...) enum ENUMNAME {__VA_ARGS__}; static unsigned int ENUMNAME ## Version () {return calcChecksum(#__VA_ARGS__);}


} /* namespace mmm */

#endif /* VERSIONEDENUM_H_ */
