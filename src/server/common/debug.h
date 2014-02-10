
#ifndef DEBUG_H_
#define DEBUG_H_

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

namespace sail
{

#define DOUT(X) std::cout << (#X) << " = \n" << (X) << "\n"
#define PFL (std::cout << "FILE " << __FILE__ << " LINE " << __LINE__ << std::endl)
#define PFLDOUT(X) PFL; DOUT(X)
#define DISPMAT(X) (dispMatrixLabeled(std::cout, (#X), X))
#define DISPMATLAB(X) (dispMatrixMatlab(std::cout, (#X), X))
#define LABEL(X) std::cout << (#X) << ": \n"; X;
#define PFLLABEL(X) PFL; LABEL(X)
#define PFLMESSAGE(X) std::cout << "FILE " << __FILE__ << " LINE " << __LINE__ << " " << X << "\n"

}
#endif /* COMMON_H_ */
