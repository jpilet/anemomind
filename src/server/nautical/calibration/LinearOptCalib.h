/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_
#define SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_

#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/math/irls.h>
#include <server/nautical/calibration/LinearCalibration.h>
#include <server/math/nonlinear/DataFit.h>
#include <Eigen/Core>

namespace sail {
namespace LinearOptCalib {


Eigen::MatrixXd orthonormalBasis(const Eigen::MatrixXd &x);
Eigen::MatrixXd makeLhs(const Eigen::MatrixXd &A, Array<Spani> spans);
Eigen::SparseMatrix<double> makeRhs(const Eigen::VectorXd &B, Array<Spani> spans);
Eigen::MatrixXd makeParameterizedApparentFlowMatrix(const Eigen::MatrixXd &A, Array<Spani> spans);
Array<Spani> makeOverlappingSpans(int dataSize, int spanSize, double relativeStep = 0.5);

void addFlowColumns(const DataFit::CoordIndexer &rows, Spani colBlock,
  std::vector<DataFit::Triplet> *dst, Eigen::VectorXd *Bopt);

Arrayi assembleIndexMap(int dstElemCount,
    Array<DataFit::CoordIndexer> dstIndexers,
  Array<Spani> srcSpans);
Eigen::VectorXd assembleVector(Arrayi indexMap, Eigen::VectorXd src);
Eigen::MatrixXd assembleMatrix(Arrayi indexMap, Eigen::MatrixXd src);

void insertDenseVectorIntoSparseMatrix(double factor, const Eigen::VectorXd &src,
  Spani dstRowSpan, int dstCol, std::vector<DataFit::Triplet> *dst);


struct Problem {
  Eigen::MatrixXd assembledA, assembledB;
  Array<Spani> rowSpansToFit;
  Spani paramColSpan;
  Spani gpsAndFlowColSpan;
  Eigen::MatrixXd Rparam;
  Eigen::SparseMatrix<double> fullProblemMatrix;
  Eigen::SparseMatrix<double> nonOrthoGpsAndFlowMatrix;
};

Problem makeProblem(const Eigen::MatrixXd &A, const Eigen::VectorXd &B,
    Array<Spani> spans);


struct Settings {
  Angle<double> inlierThresh;
  irls::Settings irlsSettings;
};

struct Results {
  Arrayd parameters;
};

Results optimize(
    const Eigen::MatrixXd &A, const Eigen::VectorXd &B,
        Array<Spani> spans,
    const Settings &settings);




}
}

#endif /* SERVER_NAUTICAL_CALIBRATION_LINEAROPTCALIB_H_ */
