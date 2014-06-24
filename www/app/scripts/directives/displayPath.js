'use strict';

angular.module('anemomindApp.directives')
.directive('displayPath', function() {

  // isolate scope
  return {
    restrict: 'E',
    link: link
  };

  function link(scope, element) {

    var lineGraph, svg, circle;

    scope.$watch('data', function(data) {

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
                                    .attr("d", lineFunction(data))
                                    .attr("stroke", "#00aaff")
                                    .attr("stroke-width", 0.5)
                                    .attr("fill", "none")
                                    .attr("stroke-opacity", 0.3);


        // group = vis.append("svg:g");
        var pathNode = lineGraph.node();

        var circle = svg.append("circle").attr({
          r: 1,
          fill: '#f33',
          transform: function () {
            var p = pathNode.getPointAtLength(scope.currentPos);
            return "translate(" + [p.x, p.y] + ")";
          }
        });

        // Let's animate the circle
        scope.timer_ret_val = false;
        var t_circle = svg.selectAll("circle");
        var last = 0;
        var t = 0;
        
        scope.startTimer();

      } // end if x undefined
    });

    scope.$watch('currentPos', function(currentPos) {
      if (currentPos !== 0) {
        var circle = d3.selectAll("circle");
        circle.attr("transform", function(d) {return "translate(" + scope.x(scope.data[currentPos].x_m) + ","+ scope.y(scope.data[currentPos].y_m)+"), scale(1)";});
      }
    });
  }
});
