#ifndef UNIFORM_H_
#define UNIFORM_H_

namespace sail
{

class Uniform
{
public:
	static void randomize();
	Uniform();
	Uniform(int count); // e.g. 2 => gen() -> [0, 2]         genInt() -> {0, 1}
	Uniform(double a, double b);
	double gen();
	int genInt();
	virtual ~Uniform();
private:
	void set(double a, double b);
	double _k, _m;
};

} /* namespace sail */
#endif /* UNIFORM_H_ */
