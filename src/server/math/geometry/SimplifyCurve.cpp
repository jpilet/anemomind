#include <server/math/geometry/SimplifyCurve.h>

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <server/common/logging.h>
#include <set>

// Implements Visvalingam’s algorithm for line simplification.
// Inspired by http://bost.ocks.org/mike/simplify/


CurveSimplifier::Triangle::Triangle(const Triangle& other) {
  LOG(FATAL) << "Triangle structure can't be copied.";
}

void CurveSimplifier::computeTriangles(std::vector<Triangle>* triangles) {
  triangles->clear();
  triangles->resize(this->numTriangles());
  int index = 0;

  for (auto it = this->_points.begin(); it != this->_points.end(); ++it) {
    Vertex* prev = 0;
    Vertex* next = 0;

    if (it == this->_points.begin()) {
      if (this->closed()) {
        prev = &(this->_points.back());
      }
    } else {
      prev = &*(it - 1);
    }

    if ((it + 1) == this->_points.end()) {
      if (this->closed()) {
        next = &*(this->_points.begin());
      }
    } else {
      next = &*(it + 1);
    }

    if (prev && next) {
      assert(index < triangles->size());
      (*triangles)[index].set(prev, &(*it), next);
      it->_triangle = &(*triangles)[index];
      ++index;
    }
  }
}


std::vector<int> CurveSimplifier::priorities() {
  std::vector<Triangle> triangles;
  computeTriangles(&triangles);

  std::set<SortingTrianglePointer> sortedTriangles;

  for (int i = 0; i < triangles.size(); ++i) {
    sortedTriangles.insert(SortingTrianglePointer(&(triangles[i])));
  }

  std::vector<int> result(_points.size());
  int priority = 0;
  if (!_closed) {
    result[0] = priority++;
    result.back() = priority++;
  }
  priority = result.size() - 1;

  // This loop will reorder the elements of "heap" so that
  // it contains curve points sorted by increasing importance.
  while (sortedTriangles.begin() != sortedTriangles.end()) {
    SortingTrianglePointer triangle = *sortedTriangles.begin();
    sortedTriangles.erase(triangle);
    result[triangle->_points[1] - &_points[0]] = priority--;

    // Removing a point changes previous and next triangles.
    if (triangle->_points[0]->_triangle) {
      Triangle* prev = triangle->_points[0]->_triangle;
      sortedTriangles.erase(prev);
      prev->_points[2] = triangle->_points[2];
      sortedTriangles.insert(prev);
    }
    if (triangle->_points[2]->_triangle) {
      Triangle* next = triangle->_points[2]->_triangle;
      sortedTriangles.erase(next);
      next->_points[0] = triangle->_points[0];
      sortedTriangles.insert(next);
    }
  }
  assert(priority == (_closed ? -1 : 1));

  return result;
}

