#include "Uniform.h"
#include <cstdlib>
#include <ctime>
#include <server/common/LineKM.h>

namespace sail
{

void Uniform::randomize()
{
	srand(time(nullptr));
}

Uniform::Uniform()
{
	set(0.0, 1.0);
}

Uniform::Uniform(int count)
{
	set(0.0, count);
}

Uniform::Uniform(double a, double b)
{
	set(a, b);
}

double Uniform::gen()
{
	return _k*rand() + _m;
}

int Uniform::genInt()
{
	return int(gen());
}

Uniform::~Uniform()
{
}


void Uniform::set(double a, double b)
{
	calcLineKM(0, RAND_MAX, a, b, _k, _m);
}


} /* namespace sail */
