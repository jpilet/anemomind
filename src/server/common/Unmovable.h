/*
 * Unmovable.h
 *
 *  Created on: 20 Oct 2016
 *      Author: jonas
 */

#ifndef SERVER_COMMON_UNMOVABLE_H_
#define SERVER_COMMON_UNMOVABLE_H_

#define MAKE_UNMOVABLE(ClassName) \
  ClassName(const ClassName &) = delete; \
  ClassName &operator=(const ClassName &) = delete;

#endif /* SERVER_COMMON_UNMOVABLE_H_ */
