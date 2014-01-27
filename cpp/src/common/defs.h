#ifndef DEFS_H_
#define DEFS_H_

namespace sail

{

#define PASSED std::cout << "PASSED: " << __FUNCTION__ << std::endl;
#define OPTWRITE(level, mes) if (settings.verbosity >= (level)) {std::cout << mes << "\n";}


#define VBWRITELN(level, message) if ((level) <= settings.verbosity) {std::cout << message << std::endl;}

#ifndef MAX
#define MAX(a, b) ((a) >= (b)? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) <= (b)? (a) : (b))
#endif

#define MMMTOL (1.0e-6)

typedef double *DoublePtr;
typedef int *IntPtr;
//typedef std::shared_ptr Ptr;
#define MPtr std::shared_ptr

#define implies(a, b) ((not (a)) or (b))
#define equivalent(a, b) ((a) == (b))

} /* namespace mmm */
#endif /* DEFS_H_ */
