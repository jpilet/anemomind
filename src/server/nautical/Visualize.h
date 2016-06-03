/*
 * Visualize.h
 *
 *  Created on: May 19, 2016
 *      Author: jonas
 *
 */

#ifndef SERVER_NAUTICAL_VISUALIZE_VISUALIZE_H_
#define SERVER_NAUTICAL_VISUALIZE_VISUALIZE_H_

#include <server/nautical/GeographicReference.h>
#include <server/plot/Plot.h>
#include <server/nautical/types/SampledSignal.h>

namespace sail {

// Plotting functions for various things we might want to plot.

RenderSettings makeDefaultTrajectoryRenderSettings();

Plottable::Ptr makeTrajectoryPlot(
    const GeographicReference &geoRef,
    const SampledSignal<GeographicPosition<double> > &positions,
    const RenderSettings &rs = makeDefaultTrajectoryRenderSettings());

}

#endif /* SERVER_NAUTICAL_VISUALIZE_VISUALIZE_H_ */
