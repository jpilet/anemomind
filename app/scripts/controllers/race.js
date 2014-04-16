'use strict';

angular.module('anemomindApp')
  .controller('RaceCtrl', function ($scope, Race, $http) {
    $scope.races = Race.get();

    $scope.loadRace = function (id) {
      d3.select("svg")
       .remove();

      $http.get('/api/races/' + id)
       .then(function(res){
          console.log('data loaded!');
          display(res.data);
        });
    };

    function display(data){

      console.log('displaying data');

      var xMin = d3.min(data, function(d) {return d.lon_rad;});
var xMax = d3.max(data, function(d) {return d.lon_rad;});
var yMin = d3.min(data, function(d) {return d.lat_rad;});
var yMax = d3.max(data, function(d) {return d.lat_rad;});

var xScale = d3.scale.linear()
                    .domain([xMin, xMax])
                    .range([0, 600]);
var yScale = d3.scale.linear()
                    .domain([yMax, yMin])
                    .range([0, 600]);
 
 //Path generator
 var lineFunction = d3.svg.line()
                          .x(function(d) { return xScale(d.lon_rad); })
                          .y(function(d) { return yScale(d.lat_rad); })
                          .interpolate("basis");

//The SVG Container
var svg = d3.select(".svgContainer").append("svg")
                           .attr("width", "100%")
                           .attr("viewBox", "0 0 600 700")
                           .attr("preserveAspectRatio", "xMidYMin")
                           .attr("style", "overflow: hidden; position: relative")
                           .attr("id", "adaptive-svg");

//Some stupid text
var text = svg.append("text")
    .text('du texte...')
    .attr("text-anchor", "middle")
    .style("font-size",'15px')
    .attr("dy",100)
    .attr("dx",100);

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