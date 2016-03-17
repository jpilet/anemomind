/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 *  NOTE: Currently, this code only load log files and make some basic checks.
 *  But I would like to turn it into something that checks the whole processing
 *  pipeline of our system (or at least the parts in C++), so that we can run
 *  it on any dataset before a major change.
 *
 *  Since I expect this to take a lot of time to run, I think it is a bad idea
 *  to implement this as unit tests.
 */

#include <device/anemobox/DispatcherUtils.h>
#include <server/common/logging.h>
#include <server/common/string.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/nautical/NavCompatibility.h>

using namespace sail;

namespace {
  Array<const char *> listPathsToLoad(int argc, const char **argv) {
    int n = argc - 1;
    Array<const char *> dst(n);
    if (n == 0) {
      std::cout <<
          "USAGE: Just pass all the paths that should be loaded, to this program"
          << std::endl;
    } else {
      for (int i = 0; i < n; i++) {
        auto p = argv[i + 1];
        std::cout << "  To analyze: " << p << std::endl;
        dst[i] = p;
      }
    }
    return dst;
  }

  struct Problem {
   std::string path, what;
   void disp(std::ostream *dst);
  };

  void Problem::disp(std::ostream *dst) {
    *dst << "   From path " << path << ": " << what << std::endl;
  }


  template <typename T>
  void checkRange(
      const char *path,
      const std::string &label,
      typename TimedSampleCollection<T>::TimedVector::const_iterator begin,
      typename TimedSampleCollection<T>::TimedVector::const_iterator end,
      std::vector<Problem> *dst) {
    int n = end - begin;
    for (int i = 0; i < n; i++) {
      TimedValue<T> sample = *(begin + i);
      if (!sample.time.defined()) {
        dst->push_back(Problem{
          path,
          stringFormat(
              "[%s] Sample with index %d has undefined time",
              label.c_str(), i)
        });
      } else if (i < n - 1) {
        TimedValue<T> next = *(begin + i + 1);
        if (next.time.defined() && sample.time > next.time) {
          dst->push_back(Problem{
            path,
            stringFormat("[%s] Sample %d and %d are not ordered",
               label.c_str(), i, i+1)
          });
        }
      }

      if (!isFinite(sample.value)) {
        dst->push_back(Problem{
          path,
          stringFormat(
              "[%s] Non-finite value at sample %d",
              label.c_str(), i)
        });
      }
    }
  }


  class DispatcherProblemVisitor {
   public:
    DispatcherProblemVisitor(
        const char *p,
        std::vector<Problem> *dst) : _path(p), _dst(dst) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
         const TimedSampleCollection<T> &coll) {
      const auto &samples = coll.samples();

      auto label = stringFormat("Channel %s of type %s", sourceName.c_str(), shortName);
      checkRange<T>(_path, label, samples.begin(), samples.end(), _dst);
    }
   private:
    const char *_path;
    std::vector<Problem> *_dst;
  };

  void checkForDispatcherProblems(const char *p, const std::shared_ptr<Dispatcher> &d,
    std::vector<Problem> *dst) {
    DispatcherProblemVisitor v(p, dst);
    visitDispatcherChannels<DispatcherProblemVisitor>(d.get(), &v);
  }

  template <typename T>
  void checkRange2(const char *path, const char *shortname, const TimedSampleRange<T> &samples,
    std::vector<Problem> *dst) {
    auto label = stringFormat("Dataset samples %s", shortname);
    const typename TimedSampleCollection<T>::TimedVector::const_iterator b = samples.begin();
    const typename TimedSampleCollection<T>::TimedVector::const_iterator e = samples.end();
    checkRange<T>(path, label, b, e, dst);
  }

  void checkForDatasetSampleProblems(const char *p, const NavDataset &ds,
      std::vector<Problem> *dst) {
#define CHECK_RANGE(handle, code, shortname, type, description) \
  checkRange2<type>(p, shortname, ds.samples<handle>(), dst);
    FOREACH_CHANNEL(CHECK_RANGE)
#undef CHECK_RANGE
  }

  void checkForNavProblems(int i, const char *p, const Nav &nav, std::vector<Problem> *dst) {
    if (!isFinite(nav.geographicPosition())) {
      dst->push_back(Problem{p,
        stringFormat("Nav with index %d has non-finite GPS position", i)});
    }
    if (!nav.time().defined()) {
      dst->push_back(Problem{p,
        stringFormat("Nav with index %d has undefined time")
      });
    }
  }

  void checkForNavProblems(const char *p, const NavDataset &ds,
      std::vector<Problem> *dst) {
    int n = NavCompat::getNavSize(ds);
    for (int i = 0; i < n; i++) {
      auto nav = NavCompat::getNav(ds, i);
      checkForNavProblems(i, p, nav, dst);
    }
  }

  void checkForDatasetProblems(const char *p, const NavDataset &ds, std::vector<Problem> *dst) {
    checkForDispatcherProblems(p, ds.dispatcher(), dst);
    checkForDatasetSampleProblems(p, ds, dst);
    checkForNavProblems(p, ds, dst);
  }

  void checkSanity(const char *path, std::vector<Problem> *dst) {
    LogLoader loader;
    loader.load(std::string(path));
    auto dataset = loader.makeNavDataset();
    checkForDatasetProblems(path, dataset, dst);
  }
}

int main(int argc, const char **argv) {
  auto paths = listPathsToLoad(argc, argv);

  if (paths.hasData()) {
    std::vector<Problem> problems;

    for (int i = 0; i < paths.size(); i++) {
      auto p = paths[i];
      std::cout << "Processing path " << i + 1 << "/" << paths.size() << ": " << p;
      checkSanity(p, &problems);
    }
    if (problems.empty()) {
      std::cout << "\n\n\nCongratulations, no problems detected!" << std::endl;
    } else {
      std::cout << "\n\n\nPROBLEM REPORT:\n";
      std::cout << problems.size() << " problems were detected:\n";
      for (auto problem: problems) {
        problem.disp(&(std::cout));
      }
    }
  }

  return 0;
}


