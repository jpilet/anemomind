/*
 * DataCalib.h
 *
 *  Created on: 24 janv. 2014
 *      Author: jonas
 */

#ifndef DATACALIB_H_
#define DATACALIB_H_

#include <server/math/armaadolc.h>
#include <server/common/Array.h>
#include <server/nautical/Nav.h>
#include <memory>
#include <vector>
#include <adolc/adouble.h>
#include <server/math/ADFunction.h>
#include <armadillo>
#include <server/math/Grid.h>


namespace sail {

class LocalRace;
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
class BoatData {
 public:
  const static int ParamCount = 4;

  BoatData(LocalRace *race, Array<Nav> navs);
  int getParamCount() const {
    return ParamCount;
  }

  int getDataCount() const {
    return _navs.size();
  }

  // These are the vectors in the local coordinate frame
  int getWindDataCount() const {
    return 2*getDataCount();
  }
  int getCurrentDataCount() const {
    return 2*getDataCount();
  }

  // Output 'getWindDataCount()' residuals to Fout, starting at index 0,
  // computed from the vector Xin
  void evalWindData(adouble *Xin, adouble *Fout);

  // Output 'getCurrentDataCount()' residuals to Fout, starting at index 0,
  // computed from the vector Xin
  void evalCurrentData(adouble *Xin, adouble *Fout);


  void fillPData(int offset, Grid3d grid, arma::umat *IJOut, arma::vec *XOut);

  void setParamOffset(int offset);

  // Outputs the initial calibration values for this boat to XOut
  void initializeParameters(double *XOut);


  // READ/WRITE access to elements of a parameter vector
  template <typename T> T &magneticCompassOffset(T *x) {return x[_paramOffset + 0];}
  template <typename T> T &windDirectionOffset(T *x) {return x[_paramOffset + 1];}
  template <typename T> T &waterSpeedCoef(T *x) {return x[_paramOffset + 2];}
  template <typename T> T &windSpeedCoef(T *x) {return x[_paramOffset + 3];}
 private:
  LocalRace *_race;
  int _paramOffset;
  Array<Nav> _navs;

  arma::advec2 calcBoatWrtEarth(const Nav &nav);
  adouble calcAwaRadians(const Nav &nav, adouble *Xin);
  adouble calcAwsMPS(const Nav &nav, adouble *Xin);
  adouble estimateHeadingRadians(const Nav &nav, adouble awaRadians, adouble *Xin);
  adouble calcWaterSpeedMPS(const Nav &nav, adouble *Xin);
};

/*
 *  Holds information for the optimization problem
 *  of fitting wind and current grids to the data
 *  collected from the sail boats.
 *
 */
class DataCalib {
 public:
  DataCalib();

  void addBoatData(std::shared_ptr<BoatData> boatData);
  virtual ~DataCalib();

  int paramCount() const {return _paramCount;}
  int windDataCount() const {return _windDataCount;}
  int currentDataCount() const {return _currentDataCount;}
  void evalWindData(adouble *Xin, adouble *Fout);
  void evalCurrentData(adouble *Xin, adouble *Fout);

  arma::sp_mat makeP(Grid3d grid);
  arma::mat makeInitialParameters();
 private:
  int _paramCount, _windDataCount, _currentDataCount, _navCount;
  std::vector<std::shared_ptr<BoatData> > _boats;
};

// A wrapper class that outputs the true wind estimates as a
// function of calibration parameters. It uses DataCalib class for this.
class WindData : public AutoDiffFunction {
 public:
  WindData(DataCalib &dataCalib) : _dataCalib(dataCalib) {}
  int inDims() {return _dataCalib.paramCount();}
  int outDims() {return _dataCalib.windDataCount();}
  void evalAD(adouble *Xin, adouble *Fout) {_dataCalib.evalWindData(Xin, Fout);}
 private:
  DataCalib &_dataCalib;
};

// A wrapper class that outputs the true current estimates as a
// function of calibration parameters. It uses DataCalib class for this.
class CurrentData : public AutoDiffFunction {
 public:
  CurrentData(DataCalib &dataCalib) : _dataCalib(dataCalib) {}
  int inDims() {return _dataCalib.paramCount();}
  int outDims() {return _dataCalib.currentDataCount();}
  void evalAD(adouble *Xin, adouble *Fout) {_dataCalib.evalCurrentData(Xin, Fout);}
 private:
  DataCalib &_dataCalib;
};

} /* namespace sail */

#endif /* DATACALIB_H_ */
