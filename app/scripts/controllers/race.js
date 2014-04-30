'use strict';

angular.module('anemomindApp')
  .controller('RaceCtrl', function ($scope, Race, $http) {
    $scope.races = Race.get();

    $scope.loadRace = function (id) {
      d3.select('svg')
       .remove();

      $http.get('/api/races/' + id)
       .then(function(res){
          console.log('race ' + id + ' loaded.');

          var xMin = d3.min(res.data, function(d) {return d.lon_rad;}),
              xMax = d3.max(res.data, function(d) {return d.lon_rad;}),
              yMin = d3.min(res.data, function(d) {return d.lat_rad;}),
              yMax = d3.max(res.data, function(d) {return d.lat_rad;});

          var low = xMin >= yMin ? yMin : xMin,
              high = xMax <= yMax ? yMax : xMax;

          var portrait = xMax - xMin <= yMax - yMin ? true : false;

          if (portrait) {
            var offset = high-xMax;
            var x = d3.scale.linear()
                    .domain([low-offset/2, xMax+offset/2])
                    .range([0, 1200]);

            var y = d3.scale.linear()
                      .domain([low, high])
                      .range([500, 0]);
          } else {
            var offset = high-yMax;
            var x = d3.scale.linear()
                      .domain([low, xMax])
                      .range([0, 1200]);

            var y = d3.scale.linear()
                      .domain([low-offset/2, yMax+offset/2])
                      .range([500, 0]);
          }
          

          display(res.data, x, y, portrait);
        });
    };

    function display(data, x, y, portrait){





 
 //Path generator
 var lineFunction = d3.svg.line()
                          .x(function(d) { return x(d.lon_rad); })
                          .y(function(d) { return y(d.lat_rad); })
                          .interpolate("basis");

//The SVG Container
var svg = d3.select(".svgContainer").append("svg")
                           .attr("width", "100%")
                           .attr("viewBox", "0 0 1200 500");

//The path !
var lineGraph = svg.append("path")
                            .attr("class", "route")
                            .attr("d", lineFunction(data))
                            .attr("stroke", "#00aaff")
                            .attr("stroke-width", 5)
                            .attr("fill", "none")
                            .attr("stroke-opacity", 0.3);


// group = vis.append("svg:g");
var pathNode = lineGraph.node();
var len = pathNode.getTotalLength();

var circle = svg.append("circle")
    .attr({
    r: 10,
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

    }
  });