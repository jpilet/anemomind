/*
 * math.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef MATH_H_
#define MATH_H_

namespace sail
{

//template <typename T, int E>
//class Expt
//{
//public:
//	inline static T eval(T x) {return x*Expt<T, E-1>(x);}
//};
//
//template <typename T>
//class Expt<T, 0>
//{
//public:
//	inline static T eval(T x) {return 1;}
//};

template <int a, int b>
class StaticPower
{
public:
	static const int result = a*StaticPower<a, b-1>::result;
};

template <int a>
class StaticPower<a, 0>
{
public:
	static const int result = 1;
};

template <typename T>
T sqr(T x) {return x*x;}

template <typename T, int dims>
void sub(T *a, T *b, T *aMinusB)
{
	for (int i = 0; i < dims; i++)
	{
		aMinusB[i] = a[i] - b[i];
	}
}

template <typename T, int dims>
T norm2(T *x)
{
	T result = 0;
	for (int i = 0; i < dims; i++)
	{
		result += sqr(x[i]);
	}
	return result;
}

template <typename T, int dims>
T norm2dif(T *a, T *b)
{
	T res[dims];
	sub<T, dims>(a, b, res);
	return norm2<T, dims>(res);
}




} /* namespace sail */

#endif /* MATH_H_ */
