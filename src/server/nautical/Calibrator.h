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
    Calibrator() : _grammar(_settings) { clear(); }
    Calibrator(const WindOrientedGrammar& grammar) : _grammar(grammar) { clear(); }

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
    void plot(GnuplotExtra *gnuplot, const std::string &title);

    //! Forget last calibration results.
    void clear();

  private:
    std::string description(std::shared_ptr<HTree> tree);
    void addAllTack(std::shared_ptr<HTree> tree);
    void addTack(int pos, double weight);
    void addBuoyTurn(std::shared_ptr<HTree> tree);

    Array<Nav> _allnavs;
    WindOrientedGrammarSettings _settings;
    WindOrientedGrammar _grammar;
    std::shared_ptr<HTree> _tree;
    ceres::Problem _problem;
    double _calibrationValues[TrueWindEstimator::NUM_PARAMS];

    // The pointers stored in this vector are owned by "_problem".
    vector<TackCost*> _maneuvers;
};

}  // namespace sail

#endif // NAUTICAL_CALIBRATOR_H
