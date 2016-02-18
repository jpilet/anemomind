#ifndef ANEMOBOX_SIMULATEBOX_H
#define ANEMOBOX_SIMULATEBOX_H

#include <string>
#include <iostream>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>

namespace sail {

bool SimulateBox(const std::string& boatDat, Array<Nav> *navs);
bool SimulateBox(std::istream& boatDat, Array<Nav> *navs);

}  // namespace sail

#endif  // ANEMOBOX_SIMULATEBOX_H
