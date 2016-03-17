/*
 *  Created on: 2016
 *      Author: Jonas Östlund <jonas@anemomind.com>
 *
 */

#include <device/anemobox/DispatcherUtils.h>
#include <server/nautical/logimport/LogLoader.h>
#include <server/common/logging.h>
#include <server/common/string.h>

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


  class DispatcherProblemVisitor {
   public:
    DispatcherProblemVisitor(
        const char *p,
        std::vector<Problem> *dst) : _path(p), _dst(dst) {}



    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
         const TimedSampleCollection<T> &coll) {

      for (int i = 0; i < coll.size(); i++) {
        auto sample = coll.samples()[i];
        if (!sample.time.defined()) {
          _dst->push_back(Problem{
            _path,
            stringFormat(
                "Sample with index %d has undefined time in channel %s of type %s",
                i, sourceName.c_str(), shortName)
          });
        }

        if (!isFinite(sample.value)) {
          _dst->push_back(Problem{
            _path,
            stringFormat(
                "Non-finite value at sample %d in channel %s of type %s",
                i, sourceName.c_str(), shortName)
          });
        }
      }

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

  void checkForDatasetProblems(const char *p, const NavDataset &ds, std::vector<Problem> *dst) {
    checkForDispatcherProblems(p, ds.dispatcher(), dst);
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
      std::cout << "Congratulations, no problems detected!" << std::endl;
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


