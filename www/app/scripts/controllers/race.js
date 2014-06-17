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
      var svg = d3.select('.svgContainer').append('svg')
                                 .attr('width', '100%')
                                 .attr('viewBox', '0 0 100 100');

      //The path !
      var lineGraph = svg.append('path')
                                  .attr('class', 'route')
                                  .attr('d', lineFunction(data))
                                  .attr('stroke', '#00aaff')
                                  .attr('stroke-width', 0.4)
                                  .attr('fill', 'none')
                                  .attr('stroke-opacity', 0.3);

      // group = vis.append('svg:g');
      var pathNode = lineGraph.node();
      var len = pathNode.getTotalLength();

      var circle = svg.append('circle').attr({
        r: 1,
        fill: '#f33',
        transform: function () {
          var p = pathNode.getPointAtLength(0);
          return 'translate(' + [p.x, p.y] + ')';
        }
      });

      // Animation code
      var orig = document.querySelector('path');

      var obj = {length:0,
                 pathLength:orig.getTotalLength()};

      orig.style.stroke = '#f60';

      var t = TweenMax.to(obj, 10, {length:obj.pathLength, onUpdate:drawLine, ease:Linear.easeNone})

      function drawLine() {
        var p = pathNode.getPointAtLength(obj.length);
        //move red circle (the boat!)
        circle.attr('transform', 'translate(' + [p.x, p.y] + ')');
        //update sensor data to be displayed
        $scope.$apply(function(){
          $scope.sensorData = 'x: ' + p.x + ', y: ' + p.y;
        });
        // orig.style.strokeDasharray = [obj.length, obj.pathLength].join(' ');
        updateSlider();
      }


      //A bunch of jQuery UI stuff

      $('#slider').slider({
        range: false,
        min: 0,
        max: 100,
        step:0.1,
        slide: function ( event, ui ) {
          t.pause();
          t.progress( ui.value/100);
          }
      }); 
          
      function updateSlider() {
        $('#slider').slider('value', t.progress()*100);
      }     

      $('#play').click(function() {
          t.play();
          if(t.progress() === 1){
            t.restart();
          }
      });
          
      $('#pause').click(function() {
          t.pause();
      });
          
      $('#reverse').click(function() {
          t.reverse();
      });
          
      $('#resume').click(function() {
          t.resume(); 
      });
      //End of animation code


    }
  });