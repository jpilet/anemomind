
#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <string>
#include "defs.h"
#include <thread>
#include <chrono>

namespace sail
{

void sleepForever();


template <typename T>
T dispLabeled(std::string label, T x)
{
	std::cout << label << " = \n" << x << "\n";
	return x;
}


#define DOUT(X) std::cout << (#X) << " = \n" << (X) << "\n"
#define PFL (std::cout << "FILE " << __FILE__ << " LINE " << __LINE__ << std::endl)
#define PFLDOUT(X) PFL; DOUT(X)
#define DISPMAT(X) (dispMatrixLabeled(std::cout, (#X), X))
#define DISPMATLAB(X) (dispMatrixMatlab(std::cout, (#X), X))
#define DISP(X) (dispLabeled((#X), X))
#define LABEL(X) std::cout << (#X) << ": \n"; X;
#define PFLLABEL(X) PFL; LABEL(X)
#define PFLMESSAGE(X) std::cout << "FILE " << __FILE__ << " LINE " << __LINE__ << " " << X << "\n"
#define FOREACH(IVAR, ARR) for (int IVAR = 0; IVAR < ARR.size(); IVAR++)
#define FOREACHMATELEM(IVAR, JVAR, MATRIX, STATEMENTS) for (int IVAR = 0; IVAR < (MATRIX).rows(); IVAR++) { for (int JVAR = 0; JVAR < (MATRIX).cols(); JVAR++) {STATEMENTS} }
#define STDFILENAME (std::string(__FUNCTION__) + ".icv")

class Exception
{
public:
	Exception(std::string file, int line, std::string msg);
	std::string getString();
private:
	int _line;
	std::string _file, _msg;
};

std::ostream &operator<<(std::ostream &s, Exception e);

#define MERROR(s) throw sail::Exception(__FILE__, __LINE__, s)
#define DEPRECATE MERROR("Code has been deprecated.")
#define MASSERT(a, b) if (!(a)) {MERROR(b);}
#define DEBUGCHECK(expr) if (expr) {PFLMESSAGE("It works");} else {MERROR("FAILED");}

typedef double *DoublePtr;


template <typename T>
bool updateLower(double cost, double &lowestCost, T value, T &bestValue)
{
	if (cost < lowestCost)
	{
		lowestCost = cost;
		bestValue = value;
		return true;
	}
	return false;
}

template <typename T>
bool updateHigher(double profit, double &highestProfit, T value, T &bestValue)
{
	if (profit > highestProfit)
	{
		highestProfit = profit;
		bestValue = value;
		return true;
	}
	return false;
}

template <typename T>
int sign(T x)
{
	if (x < 0)
	{
		return -1;
	}
	else if (x > 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

template <typename T>
void copy(int count, T *src, T *dst)
{
	for (int i = 0; i < count; i++)
	{
		dst[i] = src[i];
	}
}

double clamp(double x, double lower, double upper);
int clamp(int x, int lower, int upper);
bool isset(int pos, int x);
bool isset1(int pos, int x);
double floatMod(double a, double b);
void HSV2RGB(double *HSV, double *RGB);
void hue2RGB(double hue, double *RGB);
void hue2RGB(double hue, uint8_t *RGB);
void RGB2GreyBytes(uint8_t *RGB, uint8_t *grey);
void calcLineKM(double x0, double x1, double y0, double y1, double &k, double &m);
double rad2deg(double rad);
double deg2rad(double deg);
int posMod(int index, int dims);
double posMod(double x, int dims);
void solveQuadratic(double a, double b, double &p, double &q);
void solveQuadratic(double a, double b, double c, double &p, double &q);
bool floatEq(double a, double b, double marg = 1.0e-9);
int toMultiple(int n, int multiple);
double secondsSince1970();

}
#endif /* COMMON_H_ */
