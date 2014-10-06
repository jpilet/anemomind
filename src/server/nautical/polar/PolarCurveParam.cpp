/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/PolarCurveParam.h>
#include <server/math/CurveParam.h>
#include <server/common/LineKM.h>
#include <server/math/PolarCoordinates.h>
#include <server/common/string.h>
#include <cassert>

namespace sail {

namespace {
  arma::mat makeRayMatrix(int vertexCount) {
    LineKM angle(0, vertexCount-1, 0.0, 2.0*M_PI);
    arma::mat A = arma::zeros(2*vertexCount, vertexCount);
    for (int i = 0; i < vertexCount; i++) {
      int rowoffs = 2*i;
      int col = i;
      Angle<double> alpha = Angle<double>::radians(angle(i));
      A(rowoffs + 0, col) = calcPolarX(true, 1.0, alpha);
      A(rowoffs + 1, col) = calcPolarY(true, 1.0, alpha);
    }
    return A;
  }
}

PolarCurveParam::PolarCurveParam() :
    _segsPerCtrlSpan(0), _ctrlCount(0),
    _mirrored(false), _paramCount(0) {
}


PolarCurveParam::PolarCurveParam(int segsPerCtrlSpan, int ctrlCount, bool mirrored) :
    _segsPerCtrlSpan(segsPerCtrlSpan), _ctrlCount(ctrlCount), _mirrored(mirrored) {
    if (mirrored) {
      _paramCount = div1(ctrlCount, 2);
    } else {
      _paramCount = ctrlCount;
    }

    MDArray2d PtempCtrl = parameterizeCurve(vertexCount(), makeAllCtrlInds(), 2, true);
    arma::mat PmatCtrl = arma::mat(PtempCtrl.getData(), PtempCtrl.rows(), PtempCtrl.cols(), false, true);
    //_Pmat = arma::kron(PmatCtrl, arma::eye(2, 2))*makeP2CMat();
    _Pmat = makeRayMatrix(vertexCount())*PmatCtrl*makeParamMat();
    assert(_Pmat.n_rows == vertexDim());
    assert(_Pmat.n_cols == paramDim());
    _curveParamToVertexIndex = LineKM(0.0, 1.0, 0, vertexCount()-1);
}

Angle<double> PolarCurveParam::ctrlAngle(int ctrlIndex) const {
  assert(0 <= ctrlIndex);
  assert(ctrlIndex < _ctrlCount);
  LineKM line(-1, _ctrlCount, 0.0, 360.0);
  return Angle<double>::degrees(line(ctrlIndex));
}

Arrayi PolarCurveParam::makeAllCtrlInds() const {
  Arrayi inds(_ctrlCount + 2);
  for (int i = 0; i < _ctrlCount + 2; i++) {
    inds[i] = ctrlToVertexIndex(i-1);
  }
  return inds;
}

arma::mat PolarCurveParam::makeP2CMat() const {
  arma::mat M = arma::zeros(2*(_ctrlCount + 2), _paramCount);
  for (int i = 0; i < _ctrlCount; i++) {
    int rowOffset = 2*(1 + i);
    int col = ctrlToParamIndex(i);
    Angle<double> alpha = ctrlAngle(i);
    M(rowOffset + 0, col) = calcPolarX(true, 1.0, alpha);
    M(rowOffset + 1, col) = calcPolarY(true, 1.0, alpha);
  }
  return M;
}

arma::mat PolarCurveParam::makeParamMat() const {
  arma::mat M = arma::zeros(_ctrlCount + 2, paramCount());
  for (int i = 0; i < _ctrlCount; i++) {
    M(i + 1, ctrlToParamIndex(i)) = 1.0;
  }
  return M;
}

int PolarCurveParam::ctrlToParamIndex(int ctrlIndex) const {
  assert(0 <= ctrlIndex);
  assert(ctrlIndex < _ctrlCount);
  if (ctrlIndex < _paramCount) {
    return ctrlIndex;
  }
  int lastParamIndex = _paramCount - 1;
  int middle = _ctrlCount/2;
  int overflow = ctrlIndex - middle;
  int result = lastParamIndex - overflow;
  assert(0 <= result);
  assert(result < _paramCount);
  return result;
}

namespace {
  MDArray2d makePolarCurvePts(Arrayd vertices, double z) {
    int vertexCount = vertices.size()/2;
    bool disp3d = !std::isnan(z);
    MDArray2d pts(vertexCount, (disp3d? 3 : 2));
    if (disp3d) {
      pts.sliceCol(2).setAll(z);
    }
    for (int i = 0; i < vertexCount; i++) {
      int offs = 2*i;
      pts(i, 0) = vertices[offs + 0];
      pts(i, 1) = vertices[offs + 1];
    }
    return pts;
  }
}


MDArray2d PolarCurveParam::makePlotData(Arrayd paramsOrVertices, double z) const {
  Arrayd vertices;
  if (paramsOrVertices.size() == paramCount()) {
    vertices = Arrayd(vertexDim());
    paramToVertices(paramsOrVertices, vertices);
  } else {
    vertices = paramsOrVertices;
  }

  MDArray2d pts = makePolarCurvePts(vertices, z);
  return pts;
}


void PolarCurveParam::initializeParameters(Arrayd dst) const {
  assert(dst.size() == paramCount());
  dst.setTo(1.0);
}

Arrayd PolarCurveParam::makeInitialParameters() const {
  Arrayd params(paramCount());
  initializeParameters(params);
  return params;
}


} /* namespace mmm */
