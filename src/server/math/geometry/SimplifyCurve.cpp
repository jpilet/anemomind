#include <server/math/geometry/SimplifyCurve.h>

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <server/common/logging.h>

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

  std::vector<SortingTrianglePointer> heap;
  heap.reserve(triangles.size());

  for (int i = 0; i < triangles.size(); ++i) {
    heap.push_back(SortingTrianglePointer(&(triangles[i])));
  }

  auto heapEnd = heap.end();
  std::make_heap(heap.begin(), heapEnd);

  // This loop will reorder the elements of "heap" so that
  // it contains curve points sorted by increasing importance.
  while (heapEnd != heap.begin()) {
    SortingTrianglePointer triangle = heap.front();

    // move the front to the back
    std::pop_heap(heap.begin(), heapEnd);

    // Reduce heap size. the back of the array contains lower priority points.
    --heapEnd;

    // Removing a point changes previous and next triangles.
    if (triangle->_points[0]->_triangle) {
      Triangle* prev = triangle->_points[0]->_triangle;
      prev->_points[2] = triangle->_points[2];
    }
    if (triangle->_points[2]->_triangle) {
      Triangle* next = triangle->_points[2]->_triangle;
      next->_points[0] = triangle->_points[0];
    }

    // Sort the heap again.
    // This make_heap call could be optimized, since only 2 elements changed.
    make_heap(heap.begin(), heapEnd);
  }

  // Output the result.
  std::vector<int> result(_points.size());
  int priority = 0;
  if (!_closed) {
    result[0] = priority++;
    result.back() = priority++;
  }

  for (SortingTrianglePointer triangle : heap) {
    result[triangle->_points[1] - &_points[0]] = priority++;
  }

  return result;
}

