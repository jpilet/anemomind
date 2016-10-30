#include <server/plot/CairoUtils.h>
#include <cairo/cairo-svg.h>
#include <server/common/LineKM.h>

using namespace sail;
using namespace sail::Cairo;

int main(int argc, const char **argv) {

  PlotUtils::Settings2d settings;
  auto surface = sharedPtrWrap(
      cairo_svg_surface_create("testplot.svg",
          settings.width, settings.height));
  auto cr = sharedPtrWrap(cairo_create(surface.get()));



  renderPlot(settings, [&](cairo_t *dst) {
    int n = 300;
    LineKM m(0, n, -400, 400);
    auto f = [](double x0) {auto x = x0/100; return 100*x*x/(x*x + 1.0);};

    cairo_move_to(dst, m(0), f(m(0)));
    for (int i = 0; i < n; i++) {
      auto x = m(i);
      auto y = f(x);
      cairo_line_to(dst, x, y);
    }
    WithLocalDeviceScale ws(dst,
        WithLocalDeviceScale::Determinant);
    cairo_stroke(dst);

  }, "Time", "Velocity", cr.get());

  return 0;
}
