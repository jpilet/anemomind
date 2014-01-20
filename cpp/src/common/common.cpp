#include "common.h"
#include <sstream>
#include <cmath>

namespace sail
{

void sleepForever()
{
	std::this_thread::sleep_for(std::chrono::seconds(30000));
}


using namespace std;

Exception::Exception(std::string file, int line, std::string msg)
{
	_file = file;
	_line = line;
	_msg = msg;
	DOUT(getString());
}

std::string Exception::getString()
{
	stringstream ss;
	ss << "FILE " << _file << " LINE " << _line << ": " << _msg;
	return ss.str();
}

std::ostream &operator<<(std::ostream &s, Exception e)
{
	s  << "EXCEPTION " << e.getString();
	return s;
}

template <typename T>
T clampgen(T x, T lower, T upper)
{
	if (x < lower)
	{
		return lower;
	}
	if (x > upper)
	{
		return upper;
	}
	return x;
}

int clamp(int x, int lower, int upper)
{
	return clampgen<int>(x, lower, upper);
}

double clamp(double x, double lower, double upper)
{
	return clampgen<double>(x, lower, upper);
}


bool isset(int pos, int x)
{
	int mask = (1 << pos);
	//DOUTI(pos);
	//DOUTI(mask);
	return (mask & x);
}

bool isset1(int pos, int x)
{
	return (isset(pos, x)? 1 : 0);
}

double floatMod(double a, double b)
{
	return a - floor(a/b)*b;
}


void HSV2RGB(double *HSV, double *RGB)
{
	// http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
	double H = floatMod(HSV[0], 360.0);
	double S = HSV[1];
	double V = HSV[2];
	double C = S*V;
	double Hprime = H/60.0;
	int Hi = (int)(floor(Hprime));
	double X1 = (Hi % 2 == 0? floatMod(Hprime, 1.0) : 1.0 - floatMod(Hprime, 1.0));
	double X = C*X1;
	double R1 = 0.0;
	double G1 = 0.0;
	double B1 = 0.0;

	//printf("   C = %f     H = %f\n", C, X);

	switch (Hi)
	{
	case 0:
		R1 = C; G1 = X; B1 = 0;
		break;
	case 1:
		R1 = X; G1 = C; B1 = 0;
		break;
	case 2:
		R1 = 0; G1 = C; B1 = X;
		break;
	case 3:
		R1 = 0; G1 = X; B1 = C;
		break;
	case 4:
		R1 = X; G1 = 0; B1 = C;
		break;
	case 5:
		R1 = C; G1 = 0; B1 = X;
		break;
	default:
		break;
	}
	double m = V - C;
	//printf("   m = %f\n", m);
	RGB[0] = R1 + m;
	RGB[1] = G1 + m;
	RGB[2] = B1 + m;
}

void hue2RGB(double hue, double *RGB)
{
	double HSV[3] = {hue, 1.0, 1.0};
	HSV2RGB(HSV, RGB);
}

void hue2RGB(double hue, uint8_t *RGB)
{
	double rgb[3];
	hue2RGB(hue, rgb);
	RGB[0] = floor(255.0*rgb[0]);
	RGB[1] = floor(255.0*rgb[1]);
	RGB[2] = floor(255.0*rgb[2]);
}

void RGB2GreyBytes(uint8_t *RGB, uint8_t *grey)
{
	*grey = (uint8_t)floor(0.299*RGB[0] + 0.587*RGB[1] + 0.114*RGB[2]);
}

void calcLineKM(double x0, double x1, double y0, double y1, double &k, double &m)
{
	k = (y1 - y0)/(x1 - x0);
	m = y0 - k*x0;
}

double rad2deg(double rad)
{
	return 180.0*rad/M_PI;
}

double deg2rad(double deg)
{
	return M_PI*deg/180.0;
}

int posMod(int index, int dims)
{
	int count = index/dims;
	return (index + dims*(4 - count)) % dims;
}

double posMod(double x, int dims)
{
	//int count = index/dims;
	//return (index + dims*(4 - count)) % dims;
	return x - dims*floor(x/dims);
}

void solveQuadratic(double a, double b, double &p, double &q)
{
	p = -a/2;
	q = sqrt(a*a/4 - b);
}

void solveQuadratic(double a, double b, double c, double &p, double &q)
{
	solveQuadratic(b/a, c/a, p, q);
}

bool floatEq(double a, double b, double marg)
{
	return abs(a - b) < marg;
}

int toMultiple(int n, int multiple)
{
	return multiple*(n/multiple);
}

// http://stackoverflow.com/questions/7960318/convert-seconds-since-1970-into-date-and-vice-versa
// http://www.cplusplus.com/reference/ctime/mktime/

}

