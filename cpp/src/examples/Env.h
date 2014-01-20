/*
 * Env.h
 *
 *  Created on: 16 janv. 2014
 *      Author: jonas
 */

#ifndef ENV_H_
#define ENV_H_

#include "../common/filesystem.h"

namespace sail
{

class SysEnv
{
public:
	SysEnv();
	virtual ~SysEnv();

	Path dataset, workspace;
};

} /* namespace sail */

#endif /* ENV_H_ */
