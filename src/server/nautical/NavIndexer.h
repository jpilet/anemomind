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
  virtual std::string makeId(const Nav &src) = 0;
  virtual std::string boatId() = 0;
};


/*
 * This is an indexer using the format that we discussed
 * over Skype on 2014-04-10: 8 hexadecimal digits for the boat id and
 * 16 hexadecimal digits for the time.
 *
 */
class BoatTimeNavIndexer : public NavIndexer {
 public:
  static std::string debuggingBoatId() {return "FFFFFFFF";}
  BoatTimeNavIndexer(std::string boatId8hexDigits,     // <-- A key in the boat database
                     std::string highestId24hexDigits); // <-- Highest id among all id's in the Nav database

  // For testing purposes:
  static BoatTimeNavIndexer makeTestIndexer();
 protected:
  virtual std::string makeId(const Nav &src);
  virtual std::string boatId() {return _boatId;}
 private:
  std::string makeIdSub(int64_t time);
  std::string _boatId, _highestId;
};



} /* namespace sail */

#endif /* NAVINDEXER_H_ */
