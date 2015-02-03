#ifndef NAUTICAL_CALIBRATOR_H
#define NAUTICAL_CALIBRATOR_H

#include <Poco/Path.h>
#include <ceres/ceres.h>
#include <memory>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <string>
#include <iostream>

namespace sail {

class TackCost;
class GnuplotExtra;


class Calibrator  {
  public:
    Calibrator() : _grammar(_settings), _verbose(false) { clear(); }
    Calibrator(const WindOrientedGrammar& grammar) : _grammar(grammar), _verbose(false) { clear(); }

    //! Attempt to load data and run the minimizer. Clears previous results.
    bool calibrate(Poco::Path dataPath, Nav::Id boatId);

    //! Run the minimizer on already loaded data. Clears previous results.
    bool calibrate(const Array<Nav>& navs,
                   std::shared_ptr<HTree> tree,
                   Nav::Id boatId);

    void saveCalibration(std::ofstream *file);

    //! Print last calibration results.
    void print();

    //! Invokes gnuplot to show some error distribution.
    void plot(GnuplotExtra *gnuplot, const std::string &title, bool external);

    //! Forget last calibration results.
    void clear();

    //! If set, calibrate() will display detailed information about the
    //  minimization. It will call gnuplot to display errors.
    void setVerbose() { _verbose = true; }

    //! Use the calibration to compute true wind on the given navigation data.
    void simulate(Array<Nav> *array) const;

    //! Returns the number of maneuvers used to fit the data.
    int maneuverCount() const {
      return _maneuvers.size();
    }

    const std::vector<TackCost*> &maneuvers() const {
      return _maneuvers;
    }

    bool segment(const Array<Nav>& navs,
                 std::shared_ptr<HTree> tree);
  private:
    std::string description(std::shared_ptr<HTree> tree);
    void addAllTack(std::shared_ptr<HTree> tree);
    void addTack(int pos, double weight);
    void addBuoyTurn(std::shared_ptr<HTree> tree);
    GnuplotExtra* initializePlot();
    void finalizePlot(GnuplotExtra* gnuplot, const ceres::Solver::Summary &summary);

    Array<Nav> _allnavs;
    WindOrientedGrammarSettings _settings;
    WindOrientedGrammar _grammar;
    std::shared_ptr<HTree> _tree;
    ceres::Problem _problem;
    double _calibrationValues[TrueWindEstimator::NUM_PARAMS];

    // The pointers stored in this vector are owned by "_problem".
    vector<TackCost*> _maneuvers;

    bool _verbose;
};

}  // namespace sail

#endif // NAUTICAL_CALIBRATOR_H
