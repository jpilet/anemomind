/*
 *  Created on: Jul 1, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef VERSIONEDENUM_H_
#define VERSIONEDENUM_H_

namespace mmm {

/*
 * To be declared in a class.
 */
#define VERSIONED_ENUM(ENUMNAME, DATA) enum ENUMNAME {DATA}; static const char *ENUMNAME ## TextRepr = #DATA;

unsigned long int calcChecksum(const char);

} /* namespace mmm */

#endif /* VERSIONEDENUM_H_ */
