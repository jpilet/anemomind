'use strict';

angular.module('anemomindApp.directives')
.directive('displayPath', function() {

  // isolate scope
  return {
    restrict: 'E',
    link: link
  };

  function link(scope, element) {

    var lineGraph, svg, ellipse;

    scope.$watch('coords', function(coords) {

      if (typeof scope.x != 'undefined') {

        //Path generator
        var lineFunction = d3.svg.line()
                                 .x(function(d) { return scope.x(d["x_m"]); })
                                 .y(function(d) { return scope.y(d["y_m"]); })
                                 .interpolate("basis");

        //The SVG Container
        svg = d3.select(".svgContainer").append("svg")
                                   .attr("width", "100%")
                                   .attr("viewBox", "0 0 100 100");

        //The path !
        lineGraph = svg.append("path")
                                    .attr("class", "route")
                                    .attr("d", lineFunction(coords))
                                    .attr("stroke", "#00aaff")
                                    .attr("stroke-width", 0.5)
                                    .attr("fill", "none")
                                    .attr("stroke-opacity", 0.3);

        var pathNode = lineGraph.node();

        var arc = d3.svg.arc()
          .innerRadius(1)
          .outerRadius(7)
          .startAngle(0)
          .endAngle(0.1)

        // True wind
        svg.append("path")
          .attr("d", arc)
          .attr("id", "twa")
          .attr("fill", "red");

        // Relative wind
        svg.append("path")
          .attr("d", arc)
          .attr("id", "awa")
          .attr("fill", "orange");

        var ellipse = svg.append("ellipse").attr({
          rx: 1.8,
          ry: 0.4,
          fill: '#f33',
          transform: function () {
            var p = pathNode.getPointAtLength(scope.currentPos);
            return "translate(" + [p.x, p.y] + ")";
          }
        });

        // Let's animate the ellipse
        scope.timer_ret_val = false;
        var t_ellipse = svg.selectAll("ellipse");
        var last = 0;
        var t = 0;
        
        scope.startTimer();

      } // end if x undefined
    });

    scope.$watch('currentPos', function(currentPos) {
      if (currentPos !== 0) {
        var ellipse = d3.selectAll("ellipse");
        var twa = d3.select("#twa");
        var awa = d3.select("#awa");
        var r = 90 + scope.data[currentPos].magHdgRad * 180.0 / Math.PI;
        // the wind angle seems visually ok, but needs to be verified..
        var twaRad = r - 90 + scope.data[currentPos].twaRad * 180.0 / Math.PI;
        var awaRad = r - 90 + scope.data[currentPos].awaRad * 180.0 / Math.PI;
        ellipse.attr("transform", function(d) {return "translate(" + scope.x(scope.coords[currentPos].x_m) + ","+ scope.y(scope.coords[currentPos].y_m)+"), scale(1), rotate(" + r + ")";});
        twa.attr("transform", function(d) {return "translate(" + scope.x(scope.coords[currentPos].x_m) + ","+ scope.y(scope.coords[currentPos].y_m)+"), scale(1), rotate(" + twaRad + ")";});
        awa.attr("transform", function(d) {return "translate(" + scope.x(scope.coords[currentPos].x_m) + ","+ scope.y(scope.coords[currentPos].y_m)+"), scale(1), rotate(" + awaRad + ")";});
      }
    });
  }
});
