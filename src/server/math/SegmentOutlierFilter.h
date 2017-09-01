/*
 * SegmentOutlierFilter.h
 *
 *  Created on: 4 Aug 2017
 *      Author: jonas
 */

#ifndef SERVER_MATH_SEGMENTOUTLIERFILTER_H_
#define SERVER_MATH_SEGMENTOUTLIERFILTER_H_

#include <server/common/indexed.h>
#include <server/math/QuadSegment.h>
#include <set>
#include <vector>

namespace sail {
namespace sof {

template <int N>
using Vec = std::array<double, N>;

template <int N>
using Pair = std::pair<double, Vec<N>>;


struct Settings {
  double regularization = 1.0;
  double difRegularization = 1.0e-6;
  double costThreshold = 100.0;
  double omissionCost = 1.0;

  int maxOutlierSegments = 2;

  int maxGap() const {return 2*maxOutlierSegments + 1;}

  int verbosityThreshold = 0;

  template <typename V>
  void visitFields(V* v) {
    v->visit("reg", regularization);
    v->visit("dif_reg", difRegularization);
    v->visit("cost_threshold", costThreshold);
    v->visit("omission_cost", omissionCost);
    v->visit("max_outlier_segments", maxOutlierSegments);
    v->visit("verbosity_threshold", verbosityThreshold);
  }

  std::ostream* log = &(std::cout);
};

struct SegmentRef {
  double position = 0;
  int segmentIndex = -1;

  SegmentRef() {}

  bool defined() const {
    return segmentIndex != -1;
  }

  SegmentRef(double p, int i)
    : position(p), segmentIndex(i) {}

  std::pair<double, int> key() const {
    return {position, segmentIndex};
  }

  bool operator<(const SegmentRef& other) const {
    return key() < other.key();
  }

  bool operator!=(const SegmentRef& other) const {
    return key() != other.key();
  }
};

struct Join {
  double costIncrease = 0;
  SegmentRef left, right;
  SegmentRef joined;

  bool defined() const {
    return joined.defined();
  }

  Join() {}

  Join(
      double c, const SegmentRef& l,
      const SegmentRef& r,
      const SegmentRef& j) :
    costIncrease(c), left(l), right(r), joined(j) {}

  std::tuple<double, SegmentRef, SegmentRef> key() const {
    return std::tuple<double, SegmentRef, SegmentRef>{costIncrease, left, right};
  }

  bool operator<(const Join& other) const {
    return key() < other.key();
  }
};

std::ostream& operator<<(std::ostream& s, const Join& j);

template <typename I>
I stepIter(I i, bool forward) {
  return forward? ++i : --i;
}

// A pointer-like object that
// will remain valid even if the underlying
// vector storage is reallocated.
template <typename T>
class VecRef {
public:
  VecRef(int i, std::vector<T>* d) : _index(i), _data(d) {}

  T& operator*() {
    return (*_data)[_index];
  }
private:
  int _index;
  std::vector<T>* _data;
};

template <typename T> VecRef<T> vecRef(int i, std::vector<T>* v) {return VecRef<T>(i, v);}

template <int N>
struct SegmentLookUp {
  struct SegmentData {

    // Applicable for composite segments
    Join join;

    QuadSegment<N> segment;
    double cost = 0;
    std::set<Join> referees;
    SegmentData() {}
    SegmentData(const QuadSegment<N>& s, double c) : segment(s), cost(c) {}

    void removeRefereesFrom(std::set<Join>* joins) {
      for (const auto& r: referees) {
        joins->erase(r);
      }
    }
  };

  SegmentLookUp(const Array<std::pair<double, std::array<double, N>>>& pts) {
    segments.reserve(2*pts.size());
    for (auto ipt: indexed(pts)) {
      refs.insert(addSegment(QuadSegment<N>::primitive(
          ipt.first, ipt.second.first, ipt.second.second)));
    }
  }

  SegmentRef addSegment(const QuadSegment<N>& seg) {
    int index = segments.size();
    segments.push_back({seg, seg.cost()});
    return SegmentRef(seg.meanX(), index);
  }

  Join makeJoin(
      const SegmentRef& a,
      const SegmentRef& b,
      const Settings& s) {
    CHECK(a != b);
    auto as = vecRef(a.segmentIndex, &segments);
    auto bs = vecRef(b.segmentIndex, &segments);

    auto currentCost = (*as).cost + (*bs).cost;

    CHECK((*as).segment.rightMost() < (*bs).segment.leftMost());
    auto newSegment = join(
        (*as).segment, (*bs).segment,
        s.regularization,
        s.difRegularization);
    CHECK(newSegment.leftMost() == (*as).segment.leftMost());
    CHECK(newSegment.rightMost() == (*bs).segment.rightMost());
    auto newRef = addSegment(newSegment);

    int omitCount = (*bs).segment.rightMost()
        - (*as).segment.leftMost() - 1;
    auto omissionCost = s.omissionCost*omitCount;
    auto newCost = segments[newRef.segmentIndex].cost + omissionCost;
    auto costIncrease = newCost - currentCost;

    auto join = Join(costIncrease, a, b, newRef);

    // So that we can reconstruct
    segments[newRef.segmentIndex].join = join;

    // Necessary? Yes, so that we can remove this join
    // in case one of the segments gets joined.
    (*as).referees.insert(join);
    (*bs).referees.insert(join);

    CHECK(0 < (*as).referees.count(join));
    CHECK(0 < (*bs).referees.count(join));

    return join;
  }

  void addJoins(
      const std::set<SegmentRef>::const_iterator& orig, int from, int to,
      const Settings& settings, std::set<Join>* joins) {
    auto iter = orig;
    int i = 0;
    while (from < i && iter != refs.begin()) { // Rewind
      iter--;
      i--;
    }
    while (i < from && iter != refs.end()) { // Step forward
      iter++;
      i++;
    }
    while (i <= to && iter != refs.end()) { // Make joins
      CHECK((i == 0) == (iter == orig));
      if (i != 0) {
        auto a = orig;
        auto b = iter;
        if (a->position > b->position) {
          std::swap(a, b);
        }
        joins->insert(makeJoin(*a, *b, settings));
        //checkConsistency(*joins);
      }
      i++;
      iter++;
    }
  }

  void executeJoin(
      const Join& join, std::set<Join>* all,
      const Settings& s) {
    CHECK(0 < refs.count(join.left));
    CHECK(0 < refs.count(join.right));
    all->erase(join);
    auto begin = refs.find(join.left);
    auto end = refs.find(join.right);
    end++;
    for (auto i = begin; i != end; i++) {
      segments[i->segmentIndex].removeRefereesFrom(all);
    }
    refs.erase(begin, end);
    refs.insert(join.joined);
    auto i = refs.find(join.joined);
    addJoins(i, -s.maxGap(), s.maxGap(), s, all);
  }

  SegmentData findGreatestSegment() {
    SegmentData d;
    for (auto r: refs) {
      const auto& s = segments[r.segmentIndex];
      if (d.segment.pointCount < s.segment.pointCount) {
        d = s;
      }
    }
    return d;
  }

  void unrollInliers(
      const std::vector<SegmentData>& initStack,
      Array<bool>* dst) {
    auto stack = initStack;
    while (!stack.empty()) {
      auto x = stack.back();
      stack.pop_back();
      if (x.join.defined()) {
        stack.push_back(segments[x.join.left.segmentIndex]);
        stack.push_back(segments[x.join.right.segmentIndex]);
      } else {
        const auto& s = x.segment;
        for (int i = s.leftMost(); i <= s.rightMost(); i++) {
          (*dst)[i] = true;
        }
      }
    }
  }

  std::vector<SegmentData> segments;
  std::set<SegmentRef> refs;

  void checkConsistency(const std::set<Join>& joins) {
    LOG(INFO) << "  ****SEGMENTS";
    for (auto r: refs) {
      const auto& sd = segments[r.segmentIndex];
      const auto& s = sd.segment;
      LOG(INFO) << "    Segment("<< r.segmentIndex <<") from " << s.leftMost() << " to " << s.rightMost()
          << " at " << s.meanX();
      for (auto r: sd.referees) {
        LOG(INFO) << "        reffed by " << r;
      }
    }
    LOG(INFO) << "  ****JOINS";
    for (auto j: joins) {
      LOG(INFO) << "    Join(" << j.left.segmentIndex <<
          ", " << j.right.segmentIndex << ")";
      CHECK(0 < refs.count(j.left));
      CHECK(0 < refs.count(j.right));
      CHECK(0 < segments[j.left.segmentIndex].referees.count(j));
      CHECK(0 < segments[j.right.segmentIndex].referees.count(j));
    }
    for (auto i = refs.begin(); i != refs.end(); i++) {
      auto j = i;
      j++;
      if (j != refs.end()) {
        CHECK(
            segments[i->segmentIndex].segment.rightMost()
            < segments[j->segmentIndex].segment.leftMost());
      }
    }
  }
};

template <int N>
Array<bool> optimize(
    const Array<Pair<N>>& points,
    const Settings& settings) {
  SegmentLookUp<N> lu(points);
  std::set<Join> joins;
  for (auto i = lu.refs.begin(); i != lu.refs.end(); i++) {
    lu.addJoins(i, 1, settings.maxGap(), settings, &joins);
  }
  if (0 < settings.verbosityThreshold) {
    *(settings.log) << "Number of points: " << points.size();
    *(settings.log) << "Number of joins: " << joins.size();
    *(settings.log) << "Number of refs: " << lu.refs.size();
  }
  while (!joins.empty()) {
    bool verbose = lu.refs.size() <= settings.verbosityThreshold;
    //lu.checkConsistency(joins);
    if (verbose) {
      *(settings.log) << "\n\n--- ITERATION";
      *(settings.log) << "\n    joins: " << joins.size();
      *(settings.log) << "\n    refs: " << lu.refs.size();
      *(settings.log) << "\n    segments: " << lu.segments.size();
    }
    const auto& join = *(joins.begin());
    if (verbose) {
      *(settings.log) << "\nCost increase of joining segments: " << join.costIncrease;
      *(settings.log) << "\nJoin(" << join.left.segmentIndex
          << ", " << join.right.segmentIndex << ")";
    }
    if (join.costIncrease > settings.costThreshold) {
      if (0 < settings.verbosityThreshold) {
        *(settings.log) << "\nCost increase exceeds threshold, break.";
      }
      break;
    }
    lu.executeJoin(join, &joins, settings);
  }
  auto greatestSegment = lu.findGreatestSegment();
  if (0 < settings.verbosityThreshold) {
    *(settings.log) << "\nNumber of inliers: "
        << greatestSegment.segment.pointCount << "/"
        << points.size();
  }
  auto mask = Array<bool>::fill(points.size(), false);
  lu.unrollInliers({greatestSegment}, &mask);
  return mask;
}

}
} /* namespace sail */

#endif /* SERVER_MATH_SEGMENTOUTLIERFILTER_H_ */
