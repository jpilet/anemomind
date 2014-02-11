#include <server/plot/gnuplot_i.hpp>
#include <server/plot/extra.h>
#include <server/common/LineKM.h>
#include <cmath>

namespace sail {



void ex001AddCircle(Gnuplot &plot) {
  plot.set_style("lines");
  int count = 300;
  LineKM line(0, count-1, 0.0, 2.0*M_PI);

  std::vector<double> X(count), Y(count), Z(count);
  for (int i = 0; i < count; i++) {
    X[i] = cos(line(i));
    Y[i] = sin(line(i));
    Z[i] = (1.0/count)*i;
  }

//	plot.set_yrange(-2, 2);
//	plot.set_xrange(-2, 2);
//	plot.set_zrange(-1, 1);
  plot.plot_xyz(X, Y, Z);
}

void example001() {
  Gnuplot plot;
  ex001AddCircle(plot);
  plot.set_xautoscale();
  plot.set_yautoscale();
  plot.set_zautoscale();
  plot.showonscreen();
  sleepForever();
}



}

int main() {
  sail::example001();
}


