/*
 * DynamicInterface.h
 *
 *  Created on: 27 Jul 2017
 *      Author: jonas
 */

#ifndef SERVER_COMMON_DYNAMICINTERFACE_H_
#define SERVER_COMMON_DYNAMICINTERFACE_H_

namespace Poco {
  namespace Dynamic {
    class Var;
  };
};

namespace sail {
  class SerializationInfo;
};

#define DYNAMIC_INTERFACE \
    Poco::Dynamic::Var toDynamic() const; \
    SerializationInfo fromDynamic(const Poco::Dynamic::Var& src);



#endif /* SERVER_COMMON_DYNAMICINTERFACE_H_ */
