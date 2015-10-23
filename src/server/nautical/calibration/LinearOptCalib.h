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

namespace PowerMethod { // Suitable for computing the smallest or greatest eigenvector

// Represents the multiplication with some matrix A:
// Y = A*X
typedef std::function<Eigen::VectorXd(Eigen::VectorXd)> MatMul;

double sinAngle(const Eigen::VectorXd &a, const Eigen::VectorXd &b);

struct Settings {
 // Stop when either maxIters are reached, or
 // the angle between the vector from one iteration
 // to the next is less than tol.
 int maxIters = 300;
 Angle<double> tol = Angle<double>::radians(1.0e-4);
};

struct Results {
  Eigen::VectorXd X;
  int iters;
};

Results computeMax(MatMul A, const Eigen::VectorXd &X, const Settings &s);
Results computeMin(MatMul A, const Eigen::VectorXd &X, const Settings &s);

}




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
  Spani qaColSpan;
  Spani qbColSpan;
  Eigen::MatrixXd Rparam;
  Eigen::SparseMatrix<double> Qab;
  Eigen::SparseMatrix<double> nonOrthoGpsAndFlowMatrix;
};

// Solve without outlier rejection. Just fit it and recover the parameters.
//Eigen::VectorXd solveBasic(const Eigen::SparseMatrix &Qab);

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
