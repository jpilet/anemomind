#ifndef NAUTICAL_CALIBRATOR_H
#define NAUTICAL_CALIBRATOR_H

#include <Poco/Path.h>
#include <ceres/ceres.h>
#include <memory>
#include <server/nautical/BasicTrueWindEstimator.h>
#include <server/nautical/grammars/Grammar001.h>
#include <string>

namespace sail {

class TackCost;

class Calibrator  {
  public:
    Calibrator() : _grammar(_settings) { clear(); }

    //! Attempt to load data and run the minimizer. Clears previous results.
    bool calibrate(Poco::Path dataPath, Nav::Id boatId);

    //! Print last calibration results.
    void print();

    //! Forget last calibration results.
    void clear();

  private:
    std::string description(std::shared_ptr<HTree> tree);
    void addAllTack(std::shared_ptr<HTree> tree);
    void addTack(int pos, double weight);
    void addBuoyTurn(std::shared_ptr<HTree> tree);

    Array<Nav> _allnavs;
    Grammar001Settings _settings;
    Grammar001 _grammar;
    std::shared_ptr<HTree> _tree;
    ceres::Problem _problem;
    double _calibrationValues[BasicTrueWindEstimator::NUM_PARAMS];

    // The pointers stored in this vector are owned by "_problem".
    vector<TackCost*> _maneuvers;
};

}  // namespace sail

#endif // NAUTICAL_CALIBRATOR_H
