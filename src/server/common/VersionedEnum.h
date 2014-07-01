/*
 *  Created on: Jul 1, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef VERSIONEDENUM_H_
#define VERSIONEDENUM_H_

namespace sail {

/*
 * To be declared in a class.
 */
#define VERSIONED_ENUM(ENUMNAME, ...) enum ENUMNAME {__VA_ARGS__}; static constexpr char *ENUMNAME ## TextRepr = #__VA_ARGS__

unsigned int calcChecksum(const char *data);

} /* namespace mmm */

#endif /* VERSIONEDENUM_H_ */
