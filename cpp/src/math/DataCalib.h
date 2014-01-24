/*
 * DataCalib.h
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#ifndef DATACALIB_H_
#define DATACALIB_H_

#include "../common/Array.h"
#include "../common/Nav.h"
#include <memory>
#include <vector>
#include <adolc/adouble.h>

namespace sail
{


/*
 * BoatData
 * Holds information used for the calibration
 *
 * Parameters:
 *  - Magnetic compass calibration (angle in radians added. Normally 0)
 *  - Wind sensor calibration (angle in radians added. Normally 0)
 *  - Water speed calibration (coef. that scales the measured speed. Normally 1)
 *  - Wind speed calibration (coef. that scales the mesaured speed. Normally 1)
 *
 * These parameters are read, starting from _paramOffset in the vector being optimized.
 */
class LocalRace;
class BoatData
{
public:
	const static int ParamCount = 4;

	BoatData(LocalRace *race, Array<Nav> navs);
	int getParamCount() {return ParamCount;}

	int getDataCount() {return _navs.size();}

	// These are the vectors in the local coordinate frame
	int getWindResidualCount() {return 2*getDataCount();}
	int getCurrentResidualCount() {return 2*getDataCount();}

	// Output 'getWindResidualCount()' residuals to Fout, starting at index 0,
	// computed from the vector Xin
	void outputWindResiduals(adouble *Xin, adouble *Fout);

	// Output 'getCurrentResidualCount()' residuals to Fout, starting at index 0,
	// computed from the vector Xin
	void outputCurrentResiduals(adouble *Xin, adouble *Fout);



	void setParamOffset(int offset);
private:
	LocalRace *_race;
	int _paramOffset;
	Array<Nav> _navs;
};

/*
 *  Specifies data for the
 *
 *
 */
class DataCalib
{
public:
	// http://en.wikipedia.org/wiki/Magnetic_declination
	DataCalib();

	void addBoatData(std::shared_ptr<BoatData> boatData);
	virtual ~DataCalib();
private:
	int _paramCount;
	std::vector<std::shared_ptr<BoatData> > _boats;
};

} /* namespace sail */

#endif /* DATACALIB_H_ */
