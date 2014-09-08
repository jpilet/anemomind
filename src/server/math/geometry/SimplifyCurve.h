// Julien Pilet, 2014
#ifndef MATH_GEOMETRY_SIMPLIFY_CURVE_H
#define MATH_GEOMETRY_SIMPLIFY_CURVE_H

#include <cmath>
#include <vector>

// A curve is an ordered collection of connected 2D points.
// The point might be closed or open.
//
// The "priorities" method returns one priority value for each point.
// The lower the value, the higher the priority.
// The idea is that drawing a curve with only points of priority lower than a
// threshold limits the number of points but preserves the curve quality as
// much as possible.
class CurveSimplifier {
 public:

  CurveSimplifier(bool closed) : _closed(closed) { }

  void addPoint(double x, double y) { _points.push_back(Vertex(x, y)); }

  std::vector<int> priorities();

  bool closed() const { return _closed; }

 private:
  struct Vertex;

  struct Triangle {
    Vertex* _points[3];

    Triangle() { set(0, 0, 0); }
    Triangle(const Triangle& other);

    void set(Vertex* a, Vertex* b, Vertex* c) {
      _points[0] = a; _points[1] = b; _points[2] = c;
    }
    double area() const {
      double x[3] = { _points[0]->_x, _points[1]->_x, _points[2]->_x };
      double y[3] = { _points[0]->_y, _points[1]->_y, _points[2]->_y };

      return fabs((x[0] - x[2]) * (y[1] - y[0])
                  - (x[0] - x[1]) * (y[2] - y[0]));
    }

    double oppositeSquaredEdgeLength() const {
      double x[2] = {_points[0]->_x, _points[2]->_x};
      double y[2] = {_points[0]->_y, _points[2]->_y};
      double xdif = x[0] - x[1];
      double ydif = y[0] - y[1];
      return xdif*xdif + ydif*ydif;
    }
  };

  struct SortingTrianglePointer {
    Triangle *triangle;

    SortingTrianglePointer(Triangle* ptr) : triangle(ptr) { }

    Triangle* operator->() { return triangle; }

    bool operator < (const SortingTrianglePointer& other) const {
      // Firstly, eliminate points with small triangle area.
      // If area is the same, eliminate points that have a long opposite edge first.
      return std::make_pair(triangle->area(), -triangle->oppositeSquaredEdgeLength())
        > std::make_pair(other.triangle->area(), -other.triangle->oppositeSquaredEdgeLength());
    }

    bool operator == (const SortingTrianglePointer& other) const {
      return triangle == other.triangle;
    }

  };

  struct Vertex {
    double _x, _y;
    Triangle* _triangle;

    Vertex(double x, double y) : _x(x), _y(y), _triangle(0) { }
  };

  void computeTriangles(std::vector<Triangle>* triangles);
  int numTriangles() const {
    return (_closed ? _points.size() : _points.size() - 2);
  }

  std::vector<Vertex> _points;
  bool _closed;
};

#endif // MATH_GEOMETRY_SIMPLIFY_CURVE_H
