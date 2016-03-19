#ifndef ANEMOBOX_SIMULATEBOX_H
#define ANEMOBOX_SIMULATEBOX_H

#include <string>
#include <iostream>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <server/nautical/NavDataset.h>

namespace sail {

// TODO: Remove these ones, and replace by the one which
// returns a NavDataset
bool SimulateBox(const std::string& boatDat, Array<Nav> *navs);
bool SimulateBox(std::istream& boatDat, Array<Nav> *navs);

NavDataset SimulateBox(std::istream& boatDat, const NavDataset &ds);

}  // namespace sail

#endif  // ANEMOBOX_SIMULATEBOX_H
