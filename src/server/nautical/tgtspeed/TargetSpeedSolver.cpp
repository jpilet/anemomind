/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArrayBuilder.h>
#include <server/nautical/tgtspeed/TargetSpeedPoint.h>
#include <server/common/string.h>
#include <armadillo>
#include <server/math/armaadolc.h>
#include <server/math/nnls.h>
#include <server/common/ScopedLog.h>
#include <server/common/ArrayIO.h>
#include <server/common/ArrayBuilder.h>
#include <server/nautical/tgtspeed/TargetSpeedSolver.h>
#include <server/common/Functional.h>


namespace sail {
namespace TargetSpeedSolver {

constexpr double minErrorKnots = 0.01;

// Returns the alpha of the
// function f(x) = alpha*x^2 + beta that
// majorizes the norm g(x) = |x|
double majorizeNorm(double x, double minv) {
  return 1.0/(2.0*thresholdCloseTo0(std::abs(x), minv));
}

double BalancedCost::eval(Velocity<double> surface, const Point &pt) const {
  double error = surface.knots() - pt.value.knots();
  return pt.stability*(std::abs(error) - error*_balance);
}

MajQuad BalancedCost::majorize(Velocity<double> surface, const Point &pt) const {
  auto error = surface - pt.value;
  auto majorizedNorm = majorizeNorm(error.knots(), minErrorKnots)
      *MajQuad::fit(pt.value.knots());
  return pt.stability*(majorizedNorm - MajQuad::linear(_balance));
}

  double getStability(bool withStability, const TargetSpeedPoint &pt) {
    if (!withStability || !pt.stability().defined()) {
      return 1.0;
    }
    return pt.stability()();
  }

  Array<Array<Point> > groupPoints(TargetSpeedParam param,
      Array<TargetSpeedPoint> points, bool withStability) {
    Array<ArrayBuilder<Point> > dst(param.totalCellCount());
    for (auto pt: points) {
      auto loc = param.calcBilinearWeights(pt.windAngle(), pt.windSpeed());
      if (loc.valid()) {
        dst[loc.cellIndex].add(
            Point{loc, pt.boatSpeed(), getStability(withStability, pt)});
      }
    }
    return toArray(map([&](ArrayBuilder<Point> builder) {return builder.get();}, dst));
  }

  arma::mat makeReg(TargetSpeedSolver::Settings settings, TargetSpeedParam p) {
    return settings.radialReg*p.assembleReg(p.makeRadialSubRegs(), settings.radialRegOrder) +
           settings.angularReg*p.assembleReg(p.makeAngularSubRegs(), settings.angularRegOrder);
  }

  arma::mat makeP(TargetSpeedParam param) {
    arma::mat P = arma::zeros(param.vertexCount(), param.paramCount());
    auto elements = param.makeNonNegativeVertexParam();
    for (auto e: elements) {
      P(e.i, e.j) = e.value;
    }
    return P;
  }

  // Represents a cost on the form X'*Q*X + X'*q
  struct QuadCost {
   arma::mat Q, q;
  };

  bool sameCell(Array<Point> points) {
    if (points.empty()) {
      return true;
    } else {
      int cell = points[0].loc.cellIndex;
      for (auto pt: points) {
        if (cell != pt.loc.cellIndex) {
          return false;
        }
      }
      return true;
    }
  }

  double evalVertex(const Arrayd &vertices, int *inds, double *weights) {
    double y = 0.0;
    for (int i = 0; i < 4; i++) {
      y += weights[i]*vertices[inds[i]];
    }
    return y;
  }


  void accumulateQ(arma::mat QSrc, arma::mat qSrc, int *inds,
      arma::mat *QOut, arma::mat *qOut) {
    arma::mat &QDst = *QOut;
    arma::mat &qDst = *qOut;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        QDst(inds[i], inds[j]) += QSrc(i, j);
      }
      qDst(inds[i], 0) += qSrc(i, 0);
    }
  }

  void majorizeForCell(const Settings &s, Array<Point> points,
      Arrayd vertices,
      arma::mat *Q, arma::mat *q) {
    if (points.empty()) {
      return;
    }
    assert(sameCell(points));
    int count = points.size();
    arma::mat A = arma::zeros(count, 4);
    arma::mat B = arma::zeros(count, 1);
    arma::mat C = arma::zeros(4, 1);
    for (int i = 0; i < count; i++) {
      auto pt = points[i];
      double vertex = evalVertex(vertices, pt.loc.inds, pt.loc.weights);
      auto maj = s.getDataCostOrDefault()->majorize(
          Velocity<double>::knots(vertex), pt);
      if (maj.linear()) {
        for (int j = 0; j < 4; j++) {
          C(j, 0) += maj.b*pt.loc.weights[j];
        }
      } else {
        auto factored = maj.factor();
        double a2 = factored.getK();
        double b2 = factored.getM();
        assert(!std::isnan(a2));
        assert(!std::isnan(b2));
        B(i, 0) = -b2;
        for (int j = 0; j < 4; j++) {
          A(i, j) = a2*pt.loc.weights[j];
        }
      }
    }
    arma::mat QCell = A.t()*A;
    arma::mat qCell = C - 2*A.t()*B;
    accumulateQ(QCell, qCell, points.first().loc.inds, Q, q);
  }

  /*
   * Data term:
   *
   * In the objective function, for every data point, we evaluate
   * the error e = S(pt) - y, where S(pt) is the projection of the
   * data point on the fitted surface (encoded as a PolarGridParam::Loc) and y
   * is the observed boat speed at that data point.
   *
   * We then compute a cost for this data point that is
   *
   *  Cost(e) = stability*(|e| + balance*e)
   *
   * This cost however is not differentiable when e = 0. Therefore we
   * employ the Majorize-Minimize algorithm and find a quadratic cost function
   * that majorizes Cost(e).
   *
   *  'stability' is a score in [0, 1] associated with the data point, that
   *  gives more weight to stable points. 'balance' controls how the surface will
   *  align itself w.r.t. all the point. A 'balance' value close to -1 means the the surface
   *  will fall below most points, A 'balance' close to 1 means that the surface will
   *  fall above most points.
   *
   *  The function below computes a majorizing quadratic function for the data term.
   *
   * */
  QuadCost majorize(const Settings &settings,
      Array<Array<Point> > points,
      Arrayd vertices) {
    arma::mat Q = arma::zeros(vertices.size(), vertices.size());
    arma::mat q = arma::zeros(vertices.size(), 1);
    for (auto pointsPerCell: points) {
      majorizeForCell(settings, pointsPerCell, vertices, &Q, &q);
    }
    return QuadCost{Q, q};
  }

// Represents the cost |A*X - B|^2
struct LsqCost {
  arma::mat A, B;
};


bool sameCost(LsqCost lsq, QuadCost quad) {
  arma::mat Q = lsq.A.t()*lsq.A;
  arma::mat q = -2*lsq.A.t()*lsq.B;
  int dim = quad.Q.n_rows;
  if (!(dim == quad.Q.n_cols && dim == quad.q.n_elem &&
        dim == Q.n_cols && dim == Q.n_rows && q.n_elem == dim
        && q.n_cols == 1 && quad.q.n_cols == 1)) {
        return false;
  }

  double tol = 1.0e-6;
  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      if (std::abs(Q(i, j) - quad.Q(i, j)) > tol) {
        return false;
      }
    }
    if (std::abs(q(i, 0) - quad.q(i, 0)) > tol) {
      return false;
    }
  }

  return true;
}

// Convert a quadratic function from the
// form X'*Q*X + X'*q to the form
//      |A*X - B|^2
LsqCost quadCostToLsqCost(QuadCost quad) {
  arma::mat vecs;
  arma::vec vals;
  arma::eig_sym(vals, vecs, quad.Q);
  arma::mat A = arma::diagmat(arma::sqrt(arma::abs(vals)))*vecs.t();
  arma::mat B = arma::solve(A.t(), arma::mat(-0.5*quad.q));
  auto lsq = LsqCost{A, B};
  assert(sameCost(lsq, quad));
  return lsq;
}

LsqCost assembleLsqCost(arma::mat P, arma::mat reg, QuadCost data) {
  arma::mat Qfull = reg + data.Q;
  arma::mat Q = P.t()*Qfull*P;
  arma::mat q = P.t()*data.q;
  return quadCostToLsqCost(QuadCost{Q, q});
}

namespace {
  constexpr double initReg = 1.0e-5;
}

Settings::Settings() :
    iters(60),
    cost(nullptr),
    radialReg(8000),
    angularReg(200),
    weightedByStability(true),
    radialRegOrder(3), // 2 or 3
    angularRegOrder(5) {}

const DataCost *Settings::getDataCostOrDefault() const {
  if (cost == nullptr) {
    static BalancedCost c;
    return &c;
  }
  return cost;
}

arma::mat armat(Arrayd X) {
  return arma::mat(X.ptr(), X.size(), 1, false, true);
}


Arrayd toArray(arma::mat &X) {
  return Arrayd(X.n_elem, X.memptr());
}

MDArray2d toMDArray(arma::mat &X) {
  assert(X.n_elem == &(X(X.n_rows-1, X.n_cols-1)) - &(X(0, 0)) + 1);
  return MDArray2d(X.n_rows, X.n_cols, toArray(X));
}

double evaluateDataCost(const Settings &s,
    Arrayd vertices, Array<Array<Point> > points) {
  double cost = 0.0;
  for (auto group: points) {
    for (auto pt: group) {
      cost += s.getDataCostOrDefault()->eval(
          Velocity<double>::knots(evalVertex(vertices,
              pt.loc.inds, pt.loc.weights)),
          pt);
    }
  }
  return cost;
}

double max(Arrayd X) {
  double x = X[0];
  for (auto e: X) {
    x = std::max(e, x);
  }
  return x;
}

Results optimize(TargetSpeedParam param,
    Array<TargetSpeedPoint> tgtPoints, Settings settings) {
  auto points = groupPoints(param, tgtPoints, settings.weightedByStability);
  auto reg = makeReg(settings, param);
  auto P = makeP(param);
  Arrayd X = param.initializeNonNegativeParams();
  for (int i = 0; i < settings.iters; i++) {
    ENTERSCOPE(stringFormat("Target speed surface fitting %d/%d", i+1, settings.iters));
    arma::mat vertexMat = P*armat(X);
    Arrayd vertices = toArray(vertexMat);
    SCOPEDMESSAGE(INFO, stringFormat(" The data cost is %.7g",
      evaluateDataCost(settings, vertices,
      points)));
    auto dataTerm = majorize(settings, points, vertices);
    auto cost = assembleLsqCost(P, reg, dataTerm);
    auto solution = NNLS::solve(toMDArray(cost.A),
        toArray(cost.B), false);
    if (!solution.successful()) {
      return Results();
    }
    X = solution.X();
  }
  arma::mat PX = P*armat(X);
  Arrayd vertices = toArray(PX).dup();
  return Results{TargetSpeedFunction(param,
      toArray(map(
      [&](double x) {
        return Velocity<double>::knots(x);
      }, vertices))), evaluateDataCost(settings, vertices, points),
      settings, vertices};
}

TuneSettings::TuneSettings() :
    changeFactor(4.0), setCount(2), chunkSize(30), reducedIters(20) {}

Array<Array<TargetSpeedPoint> > splitPoints(TuneSettings settings, Array<TargetSpeedPoint> pts) {
  Array<ArrayBuilder<TargetSpeedPoint> > splits(settings.setCount);
  int n = pts.size();
  for (int i = 0; i < n; i++) {
    int dstIndex = (i/settings.chunkSize) % settings.setCount;
    splits[dstIndex].add(pts[i]);
  }
  return toArray(map([&](ArrayBuilder<TargetSpeedPoint> bd) {
    return bd.get();
  }, splits));
}

Settings makeSettingsAtGridPoint(TuneSettings tuneSettings, Settings settings, Arrayi inds) {
  double logChange = log(tuneSettings.changeFactor);
  settings.angularReg = settings.angularReg*exp(logChange*inds[0]);
  settings.radialReg = settings.radialReg*exp(logChange*inds[1]);
  return settings;
}

Settings optimizeParameters(TargetSpeedParam param,
    Array<TargetSpeedPoint> allPoints, TuneSettings tuneSettings,
      Settings initSettings) {
  ENTER_FUNCTION_SCOPE;
  Settings settings = initSettings;
  auto splits = splitPoints(tuneSettings, allPoints);


  Array<Array<Point> > processedPoints = groupPoints(param,
        allPoints, initSettings.weightedByStability);

  // This is the objective function that we optimize in
  // order to tune the parameters
  GridSearch::Objf objf = [&](Arrayi inds) {
    std::stringstream ss;
    ss << "Evaluate the objective function at (" << inds[0] << ", " << inds[1] << ")";
    ENTERSCOPE(ss.str());
    auto settings = makeSettingsAtGridPoint(tuneSettings, initSettings, inds);
    settings.iters = std::min(tuneSettings.reducedIters, settings.iters);
    SCOPEDMESSAGE(INFO, stringFormat("  Angular reg: %.6g", settings.angularReg));
    SCOPEDMESSAGE(INFO, stringFormat("   Radial reg: %.6g", settings.radialReg));

    // Optimize the parameters over different subsets of the data points.
    auto splitResults = map([&](Array<TargetSpeedPoint> subset) {
      return optimize(param, subset, settings);
    }, splits).toArray();

    // Now use the various parameters and see how
    // well they generalize, by computing the data cost
    // on the full set.
    double totalCost = 0;
    for (auto result: splitResults) {
      totalCost += evaluateDataCost(settings, result.rawVerticesKnots, processedPoints);
    }
    SCOPEDMESSAGE(INFO, stringFormat("  The objective function evaluates to %.6g", totalCost));
    return totalCost;
  };
  Arrayi Xinit{0, 0};
  return makeSettingsAtGridPoint(
      tuneSettings, settings, GridSearch::minimize<2>(
          objf, Xinit, tuneSettings.gridSearchSettings));

}










}
}
