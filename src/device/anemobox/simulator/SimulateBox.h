#ifndef ANEMOBOX_SIMULATEBOX_H
#define ANEMOBOX_SIMULATEBOX_H

#include <string>
#include <iostream>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <server/nautical/NavDataset.h>

namespace sail {

NavDataset SimulateBox(const std::string& boatDat, const NavDataset &ds);
NavDataset SimulateBox(std::istream& boatDat, const NavDataset &ds);

}  // namespace sail

#endif  // ANEMOBOX_SIMULATEBOX_H
