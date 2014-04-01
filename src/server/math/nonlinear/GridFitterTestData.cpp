/*
 *  Created on: 13 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GridFitterTestData.h"

namespace sail {

GridFitterTestData::GridFitterTestData() {
  const int sampleCount_ = 30;
  const double Xdata[sampleCount_] = {-0.967399, -0.782382, -0.740419, -0.725537, -0.716795, -0.686642, -0.604897, -0.563486, -0.514226, -0.444451, -0.329554, -0.270431, -0.211234, -0.198111, -0.0452059, 0.0268018, 0.10794, 0.213938, 0.257742, 0.271423, 0.434594, 0.536459, 0.566198, 0.59688, 0.608354, 0.680375, 0.823295, 0.83239, 0.904459, 0.997849};
  const double Ygtdata[sampleCount_] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const double Ynoisydata[sampleCount_] = {-0.994827, -0.864355, -0.954944, -1.08159, -0.944979, -0.990285, -1.00257, -0.81089, -1.08299, -0.891457, -0.989302, -0.892034, -1.03991, -0.843388, -1.08667, 0.940983, 1.12309, 1.16761, 0.827902, 1.17973, 1.0104, 0.834422, 0.876886, 1.06529, 1.15609, 0.939557, 0.825669, 0.808009, 0.983081, 0.825238};
  X = Arrayd(sampleCount_, Xdata);
  Ygt = Arrayd(sampleCount_, Ygtdata);
  Ynoisy = Arrayd(sampleCount_, Ynoisydata);
  sampleCount = sampleCount_;

  const bool splitdata0[sampleCount_] = {1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};
  const bool splitdata1[sampleCount_] = {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0};
  const bool splitdata2[sampleCount_] = {0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1};
  const bool splitdata3[sampleCount_] = {1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1};
  const bool splitdata4[sampleCount_] = {0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1};
  const bool splitdata5[sampleCount_] = {1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1};
  const bool splitdata6[sampleCount_] = {0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0};
  const bool splitdata7[sampleCount_] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0};
  const bool splitdata8[sampleCount_] = {1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0};

  const int splitCount = 9;
  const Arrayb splitsdata[splitCount] = {
      Arrayb(sampleCount_, splitdata0),
      Arrayb(sampleCount_, splitdata1),
      Arrayb(sampleCount_, splitdata2),
      Arrayb(sampleCount_, splitdata3),
      Arrayb(sampleCount_, splitdata4),
      Arrayb(sampleCount_, splitdata5),
      Arrayb(sampleCount_, splitdata6),
      Arrayb(sampleCount_, splitdata7),
      Arrayb(sampleCount_, splitdata8)};
   splits = Array<Arrayb>(splitCount, splitsdata);
}

} /* namespace sail */
