/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/common/Uniform.h>
#include <server/math/Filter1d.h>
#include <server/plot/extra.h>

int main() {
  using namespace sail;
  int middle = 400;
  int count = 2*middle;
  Arrayd Ygt(count);
  Arrayd Ynoisy(count);
  Uniform rng(-1, 1);
  Arrayd inds(count);
  for (int i = 0; i < count; i++) {
    inds[i] = i;
    Ygt[i] = (i < middle? 0 : 1);
    Ynoisy[i] = Ygt[i] + rng.gen();
  }

  double ir = 100;
  Arrayd Yfiltered = filter1d(Ynoisy, Arrayi::args(1, 2), Arrayd::args(ir, ir), Array<Arrayb>(4));

  GnuplotExtra plot;
  plot.set_style("lines");
  plot.plot_xy(inds, Ygt, "Ground truth");
  plot.plot_xy(inds, Ynoisy, "Noisy signal");
  plot.plot_xy(inds, Yfiltered, "Filtered signal");
  plot.show();


  return 0;
}


