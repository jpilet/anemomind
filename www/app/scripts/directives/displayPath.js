'use strict';

angular.module('anemomindApp.directives')
.directive('displayPath', function() {

  // isolate scope
  return {
    restrict: 'E',
    link: link
  };

  function link(scope, element) {

    var lineGraph, svg, ellipse, boatGroup, r;

    scope.$watch('coords', function(coords) {

      if (typeof scope.x != 'undefined') {

        var zoom = d3.behavior.zoom()
          .x(scope.x)
          .y(scope.y)
          .scaleExtent([1, 10])
          .on("zoom", function() {
            lineGraph.attr("transform", "translate(" + d3.event.translate + ") scale(" + d3.event.scale + ")");
            boatGroup.attr("transform", function(d) {return "translate(" + scope.x(scope.coords[scope.currentPos].x_m) + ","+ scope.y(scope.coords[scope.currentPos].y_m)+"), rotate(" + r + ")";});
          });

        //Path generator
        var lineFunction = d3.svg.line()
                                 .x(function(d) { return scope.x(d["x_m"]); })
                                 .y(function(d) { return scope.y(d["y_m"]); })
                                 .interpolate("basis");

        //The SVG Container
        svg = d3.select(".svgContainer").append("svg")
                                   .attr("width", "100%")
                                   .attr("viewBox", "0 0 100 100")
                                   .call(zoom);

        //The path !
        lineGraph = svg.append("path")
                                    .attr("class", "route")
                                    .attr("d", lineFunction(coords))
                                    .attr("stroke", "#00aaff")
                                    .attr("stroke-width", 0.15)
                                    .attr("fill", "none")
                                    .attr("stroke-opacity", 0.3);

        var pathNode = lineGraph.node();

        boatGroup = svg.append("g");

        var arc = d3.svg.arc()
          .innerRadius(1)
          .outerRadius(7)
          .startAngle(0)
          .endAngle(0.1)

        // True wind
        boatGroup.append("path")
          .attr("d", arc)
          .attr("id", "twa")
          .attr("fill", "red");

        // Relative wind
        boatGroup.append("path")
          .attr("d", arc)
          .attr("id", "awa")
          .attr("fill", "orange");

        var ellipse = boatGroup.append("ellipse").attr({
          rx: 1.8,
          ry: 0.4,
          fill: '#f33'
        });
        
        scope.startTimer();

      } // end if x undefined
    });

    scope.$watch('currentPos', function(currentPos) {
      if (currentPos !== 0) {
        var ellipse = d3.selectAll("ellipse");
        var twaArrow = d3.select("#twa");
        var awaArrow = d3.select("#awa");
            r = 90 + scope.data[currentPos].magHdgRad * 180.0 / Math.PI;
        // the wind angle seems visually ok, but needs to be verified..
        var twa = scope.data[currentPos].externalTwaRad * 180.0 / Math.PI - 90;
        var awa = scope.data[currentPos].awaRad * 180.0 / Math.PI - 90;
        boatGroup.attr("transform", function(d) {return "translate(" + scope.x(scope.coords[currentPos].x_m) + ","+ scope.y(scope.coords[currentPos].y_m)+"), rotate(" + r + ")";});
        if (!isNaN(twa)) {
          twaArrow.attr("transform", function(d) {return "rotate(" + twa + ")";});
        }
        if (!isNaN(awa)) {
          awaArrow.attr("transform", function(d) {return "rotate(" + awa+ ")";});
        }
      }
    });
  }
});
