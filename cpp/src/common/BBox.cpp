#include <iostream>
#include "BBox.h"

namespace sail
{

std::ostream &operator<< (std::ostream &s, BBox3d box)
{
	s << "BBox3d( " << box.getSpan(0) << ", " << box.getSpan(1) << ", " << box.getSpan(2) << ")" << std::endl;
	return s;
}


std::ostream &operator<< (std::ostream &s, BBox2d box)
{
	s << "BBox2d( " << box.getSpan(0) << ", " << box.getSpan(1) << ")" << std::endl;
	return s;
}




}
