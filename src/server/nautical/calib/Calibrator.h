#ifndef NAUTICAL_CALIBRATOR_H
#define NAUTICAL_CALIBRATOR_H

#include <Poco/Path.h>
#include <ceres/ceres.h>
#include <memory>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <string>
#include <iostream>
#include <device/Arduino/libraries/Corrector/Corrector.h>
#include <server/nautical/FlowErrors.h>

namespace sail {

class TackCost;
class GnuplotExtra;

struct ManeuverData {
  TimeStamp time;
  double angleError;
};

class Calibrator  {
  public:
    Calibrator() : _grammar(_settings), _verbose(false) { clear(); }
    Calibrator(const WindOrientedGrammar& grammar) : _grammar(grammar), _verbose(false) { clear(); }

    //! Attempt to load data and run the minimizer. Clears previous results.
    bool calibrate(Poco::Path dataPath, Nav::Id boatId);

    //! Run the minimizer on already loaded data. Clears previous results.
    bool calibrate(const NavDataset& navs,
                   std::shared_ptr<HTree> tree,
                   Nav::Id boatId);

    void saveCalibration(std::ostream *file) const;

    //! Print last calibration results.
    void print() const;

    //! Write a matlab compatible file with all maneuvers.
    bool saveResultsAsMat(const char *filename) const;

    //! Invokes gnuplot to show some error distribution.
    void plot(GnuplotExtra *gnuplot, const std::string &title, bool external);

    //! Forget last calibration results.
    void clear();

    void initializeParameters();

    //! If set, calibrate() will display detailed information about the
    //  minimization. It will call gnuplot to display errors.
    void setVerbose() { _verbose = true; }

    //! Use the calibration to compute true wind on the given navigation data.
    NavDataset simulate(const NavDataset &array) const;

    //! Returns the number of maneuvers used to fit the data.
    int maneuverCount() const {
      return _maneuvers.size();
    }

    const std::vector<TackCost*> &maneuvers() const {
      return _maneuvers;
    }

    bool segment(const NavDataset& navs,
                 std::shared_ptr<HTree> tree);

    WindOrientedGrammar grammar() const;

    Array<TimeStamp> maneuverTimeStamps() const;
    Array<ManeuverData> maneuverData() const;
  private:
    std::string description(std::shared_ptr<HTree> tree);
    void addAllTack(std::shared_ptr<HTree> tree);
    void addTack(int pos, double weight);
    void addBuoyTurn(std::shared_ptr<HTree> tree);
    GnuplotExtra* initializePlot();
    void finalizePlot(GnuplotExtra* gnuplot, const ceres::Solver::Summary &summary);

    NavDataset _allnavs;
    WindOrientedGrammarSettings _settings;
    WindOrientedGrammar _grammar;
    std::shared_ptr<HTree> _tree;
    ceres::Problem _problem;
    double _calibrationValues[TrueWindEstimator::NUM_PARAMS];

    // The pointers stored in this vector are owned by "_problem".
    vector<TackCost*> _maneuvers;

    bool _verbose;
};

// Calibrates a Corrector<double> by using a Calibrator.
// This lets us recover both true wind and current.
Corrector<double> calibrateFull(Calibrator *calib,
    const NavDataset& navs,
    std::shared_ptr<HTree> tree,
    Nav::Id boatId);


Corrector<double> calibrateFull(Calibrator *calib,
    const NavDataset& navs,
    Nav::Id boatId);

WindCurrentErrors computeErrors(Calibrator *calib, Corrector<double> corr);


}  // namespace sail


#endif // NAUTICAL_CALIBRATOR_H
