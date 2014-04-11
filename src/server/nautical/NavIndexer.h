/*
 *  Created on: 2014-04-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVINDEXER_H_
#define NAVINDEXER_H_

#include <server/nautical/Nav.h>
#include <string>

namespace sail {

/*
 * Responsible for generating a unique
 * id for a new Nav and assigning a boat id to it.
 *
 * I think it is a reasonable requirement that any Nav must have an id and a boat-id
 * before it is inserted in a database.
 */
class NavIndexer {
 public:
  Nav make(const Nav &src);
  virtual ~NavIndexer() {}
 protected:
  virtual Nav::Id makeId(const Nav &src) = 0;
  virtual Nav::Id boatId() = 0;
};


/*
 * This is an indexer using the format that we discussed
 * over Skype on 2014-04-10: 8 hexadecimal digits for the boat id and
 * 16 hexadecimal digits for the time.
 *
 */
class BoatTimeNavIndexer : public NavIndexer {
 public:
  static Nav::Id debuggingBoatId() {return "FFFFFFFF";}
  BoatTimeNavIndexer(Nav::Id boatId8hexDigits,     // <-- A key in the boat database
                     Nav::Id highestId24hexDigits); // <-- Highest id among all id's in the Nav database

  // For testing purposes:
  static BoatTimeNavIndexer makeTestIndexer();
 protected:
  virtual Nav::Id makeId(const Nav &src);
  virtual Nav::Id boatId() {return _boatId;}
 private:
  Nav::Id makeIdSub(int64_t time);
  Nav::Id _boatId, _highestId;
};



} /* namespace sail */

#endif /* NAVINDEXER_H_ */
