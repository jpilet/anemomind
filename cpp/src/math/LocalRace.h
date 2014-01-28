/*
 * LocalRace.h
 *
 *  Created on: 17 janv. 2014
 *      Author: jonas
 */

#ifndef LOCALRACE_H_
#define LOCALRACE_H_

#include "mathutils.h"
#include "../common/Array.h"
#include "../common/Nav.h"
#include "../common/BBox.h"
#include "Grid.h"

namespace sail
{

class LocalRace
{
public:
	LocalRace();
	virtual ~LocalRace();

	// Constructor multiple boats
	LocalRace(Array<Array<Nav> > navs, double spaceStep, double timeStep);

	// Constructor a single boat
	LocalRace(Array<Nav> navs, double spaceStep, double timeStep);

	arma::vec2 calcNavLocalPos(Nav nav);
	arma::vec3 calcNavLocalPosAndTime(Nav nav);
	MDArray2d calcNavsLocalPosAndTime(Array<Nav> navs);
	double calcNavLocalTime(const Nav &nav);
	Array<arma::vec2> calcNavsLocalPos(Array<Nav> navs);
	void makeSpatioTemporalPlot(Array<Nav> navs);
	Grid3d &getWindGrid();
	Grid3d &getCurrentGrid();

	void setMagDecl(double magDeclRadians);
	double getMagDecl() const {return _magDeclRadians;}
private:
	void initialize(Array<Nav> navs, double spaceStep, double timeStep);

	// See calcNavLocalPos to see how these are used to map to the local coordinate system.
	arma::vec3 _cog;   // Origin 3d coordinates
	double _timeOffset;
	arma::mat23 _axes; // Axes of local coordinate system, stored as rows

	double _magDeclRadians;
	Grid3d _wind, _current;

	BBox3d calcBBoxXYTimeWithoutOffsetDuringInitialization(Array<Nav> navs);
};

} /* namespace sail */

#endif /* LOCALRACE_H_ */
