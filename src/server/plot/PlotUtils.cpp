/*
 * PlotUtils.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: jonas
 */

#include <server/plot/PlotUtils.h>
#include <assert.h>
#include <server/common/math.h>
#include <server/common/LineKM.h>

namespace sail {
namespace PlotUtils {

BBox3d makeDefaultBBox() {
  BBox3d dst;
  double a[3] = {0, 0, 0};
  double b[3] = {1, 1, 1};
  dst.extend(a);
  dst.extend(b);
  return dst;
}

bool isEmpty(const BBox3d &box) {
  double diag = 0.0;
  for (int i = 0; i < 3; i++) {
    double w = box.getSpan(i).width();
    diag += sqr(w);
  }
  return diag < 1.0e-12;
}


BBox3d ensureGoodBBox(
    const BBox3d &bbox,
    const Settings2d &settings) {
  return isEmpty(bbox)? settings.defaultBBox : bbox;
}

Eigen::Vector4d getCorner(const BBox3d &box, int cornerIndex0) {
  Eigen::Vector4d offset = Eigen::Vector4d::Zero();
  offset(3) = 1.0;
  int cornerIndex = cornerIndex0;
  for (int i = 0; i < 3; i++) {
    int rem = cornerIndex % 2;
    const auto &sp = box.getSpan(i);
    offset(i) = rem == 0? sp.minv() : sp.maxv();
    cornerIndex = cornerIndex/2;
  }
  assert(cornerIndex == 0);
  return offset;
}

BBox3d projectBBox(const Eigen::Matrix4d &pose, const BBox3d &box) {
  BBox3d dst;
  for (int i = 0; i < 8; i++) {
    Eigen::Vector4d corner = getCorner(box, i);
    Eigen::Vector4d transformed = pose*corner;
    dst.extend(transformed.data());
  }
  return dst;
}


Eigen::Matrix<double, 2, 4> computeTotalProjection(
    const BBox3d &a, const Settings2d &settings) {

  // Assuming homogeneous 3d coordinates (for future 3d rendering)
  Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();

  if (!settings.axisIJ) {
    pose(1, 1) = -1;
    pose(2, 2) = -1; // So that z = cross(x, y) instead of z = -cross(x, y)
  }


  auto proj = projectBBox(pose, ensureGoodBBox(a, settings));

  LineKM rawXFit(proj.getSpan(0).minv(), proj.getSpan(0).maxv(),
      settings.xMargin, settings.width - settings.xMargin);
  LineKM rawYFit(proj.getSpan(1).minv(), proj.getSpan(1).maxv(),
      settings.yMargin, settings.height - settings.yMargin);

  // Assuming homogeneous 3d coordinates. But here, the z coordinate will just
  // disappear.
  Eigen::Matrix<double, 2, 4> viewMat = Eigen::Matrix<double, 2, 4>::Zero();

  int tcol = 3;
  if (settings.orthonormal) {
    double minScale = std::min(rawXFit.getK(), rawYFit.getK());
    Eigen::Vector2d srcMiddle(
        proj.getSpan(0).middle(),
        proj.getSpan(1).middle());
    Eigen::Vector2d dstMiddle(
        0.5*settings.width,
        0.5*settings.height);

    Eigen::Vector2d translate = dstMiddle - minScale*srcMiddle;
    viewMat(0, 0) = minScale; viewMat(0, tcol) = translate(0);
    viewMat(1, 1) = minScale; viewMat(1, tcol) = translate(1);
  } else {
    viewMat(0, 0) = rawXFit.getK(); viewMat(0, tcol) = rawXFit.getM();
    viewMat(1, 1) = rawYFit.getK(); viewMat(1, tcol) = rawYFit.getM();
  }
  return viewMat*pose;
}

HSV HSV::fromHue(Angle<double> h) {
  HSV dst;
  dst.hue = h;
  dst.saturation = 1.0;
  dst.value = 1.0;
  return dst;
}

RGB hsv2rgb(const HSV &in) {
    double      hh, p, q, t, ff;
    long        i;
    RGB         out;

    if(in.saturation <= 0.0) {       // < is bogus, just shuts up warnings
        out.red = in.value;
        out.green = in.value;
        out.blue = in.value;
        return out;
    }
    hh = in.hue.degrees();
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.value * (1.0 - in.saturation);
    q = in.value * (1.0 - (in.saturation * ff));
    t = in.value * (1.0 - (in.saturation * (1.0 - ff)));

    switch(i) {
    case 0:
        out.red = in.value;
        out.green = t;
        out.blue = p;
        break;
    case 1:
        out.red = q;
        out.green = in.value;
        out.blue = p;
        break;
    case 2:
        out.red = p;
        out.green = in.value;
        out.blue = t;
        break;

    case 3:
        out.red = p;
        out.green = q;
        out.blue = in.value;
        break;
    case 4:
        out.red = t;
        out.green = p;
        out.blue = in.value;
        break;
    case 5:
    default:
        out.red = in.value;
        out.green = p;
        out.blue = q;
        break;
    }
    return out;
}

}
} /* namespace sail */
