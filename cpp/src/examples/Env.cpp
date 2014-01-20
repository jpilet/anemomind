/*
 * Env.cpp
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#include "Env.h"
#include <assert.h>
#include <iostream>

namespace sail
{

SysEnv::SysEnv()
{
	if (exists("/home/jonas"))
	{
		Path data("/home/jonas/data");
		workspace = data.cat("workspace").cat("cpp");
		dataset = data.cat("datasets").cat("irene");
	}
	else
	{
		std::cerr << "Please configure" << std::endl;
		assert(false);
	}
}

SysEnv::~SysEnv()
{
	// TODO Auto-generated destructor stub
}

} /* namespace sail */
