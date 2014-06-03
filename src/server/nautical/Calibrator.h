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
class GnuplotExtra;

class Calibrator  {
  public:
    Calibrator() : _grammar(_settings) { clear(); }

    //! Attempt to load data and run the minimizer. Clears previous results.
    bool calibrate(Poco::Path dataPath, Nav::Id boatId);

    //! Attempt to run the minimizer. Clears previous results.
    bool calibrate(Array<Nav> navs);

    //! Print last calibration results.
    void print();

    //! Invokes gnuplot to show some error distribution.
    void plot(GnuplotExtra *gnuplot, const std::string &title);

    //! Forget last calibration results.
    void clear();


    /*
     * Read accessors.
     */
    //! Return all navs.
    Array<Nav> allnavs() const {return _allnavs;}

    //! Return parse tree.
    std::shared_ptr<HTree> tree() const {return _tree;}

    //! Return the grammar
    const Grammar001 &grammar() const {return _grammar;}

    //! Return pointer to calibration values.
    const double *calibrationValues() const {return _calibrationValues;}
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
