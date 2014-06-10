'use strict';

angular.module('anemomindApp')
  .controller('RaceCtrl', function ($scope, Race, $http) {
    $scope.races = Race.get();

    $scope.loadRace = function (id) {
      d3.select('svg').remove();

      $http.get('/api/races/' + id).then(function (res) {
        console.log('race ' + id + ' loaded with origin: [' + res.data.origin.x + ',' + res.data.origin.y + '].' );

        var xMin = d3.min(res.data.coords, function(d) {return d["x_m"];});
        var xMax = d3.max(res.data.coords, function(d) {return d["x_m"];});
        var yMin = d3.min(res.data.coords, function(d) {return d["y_m"];});
        var yMax = d3.max(res.data.coords, function(d) {return d["y_m"];});
        var low = Math.min(xMin, yMin);
        var high = Math.max(xMax, yMax);

        var portrait = xMax - xMin <= yMax - yMin;

        if (portrait) {
          var offset = high-xMax;
          var x = d3.scale.linear()
                    .domain([low-offset/2, xMax+offset/2])
                    .range([0, 100]);

          var y = d3.scale.linear()
                    .domain([low, high])
                    .range([100, 0]);
        } else {
          var offset = high-yMax;
          var x = d3.scale.linear()
                    .domain([low, high])
                    .range([0, 100]);

          var y = d3.scale.linear()
                    .domain([low-offset/2, yMax+offset/2])
                    .range([100, 0]);
        }

        display(res.data.coords, x, y, portrait);
      });
    };

    function display(data, x, y, portrait){

      //Path generator
      var lineFunction = d3.svg.line()
                               .x(function(d) { return x(d["x_m"]); })
                               .y(function(d) { return y(d["y_m"]); })
                               .interpolate("basis");

      //The SVG Container
      var svg = d3.select(".svgContainer").append("svg")
                                 .attr("width", "100%")
                                 .attr("viewBox", "0 0 100 100");

      //The path !
      var lineGraph = svg.append("path")
                                  .attr("class", "route")
                                  .attr("d", lineFunction(data))
                                  .attr("stroke", "#00aaff")
                                  .attr("stroke-width", 1)
                                  .attr("fill", "none")
                                  .attr("stroke-opacity", 0.3);

      // group = vis.append("svg:g");
      var pathNode = lineGraph.node();
      var len = pathNode.getTotalLength();

      var circle = svg.append("circle").attr({
        r: 1,
        fill: '#f33',
        transform: function () {
          var p = pathNode.getPointAtLength(0)
          return "translate(" + [p.x, p.y] + ")";
        }
      });

      var duration = 10000;
      circle.transition()
            .duration(duration)
            .ease("linear")
            .attrTween("transform", function (d, i) {
              return function (t) {
                  var p = pathNode.getPointAtLength(len*t);
                  return "translate(" + [p.x, p.y] + ")";
              }
            });
    };
  });