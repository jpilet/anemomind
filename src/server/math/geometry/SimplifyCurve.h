// Julien Pilet, 2014
#ifndef MATH_GEOMETRY_SIMPLIFY_CURVE_H
#define MATH_GEOMETRY_SIMPLIFY_CURVE_H

#include <vector>

struct Triangle;

// Internal class used for intermediate computation.
struct Vertex {
  double _x, _y;
  Triangle* _triangle;

  Vertex(double x, double y) : _x(x), _y(y), _triangle(0) { }
};

// A Curve is an ordered collection of connected 2D points.
// The point might be closed or open.
//
// The "priorities" method returns one priority value for each point.
// The lower the value, the higher the priority.
// The idea is that drawing a curve with only points of priority lower than a
// threshold limits the number of points but preserves the curve quality as
// much as possible.
class Curve {
 public:

  Curve(bool closed) : _closed(closed) { }

  void addPoint(double x, double y) { _points.push_back(Vertex(x, y)); }

  std::vector<int> priorities();

  bool closed() const { return _closed; }

 private:
  void computeTriangles(std::vector<Triangle>* triangles);
  int numTriangles() const {
    return (_closed ? _points.size() : _points.size() - 2);
  }

  std::vector<Vertex> _points;
  bool _closed;
};

#endif // MATH_GEOMETRY_SIMPLIFY_CURVE_H
