'use strict';

angular.module('anemomindApp')
  .controller('RaceCtrl', function ($scope, Race, $http, $log) {
    $scope.races = Race.get();

    $scope.loadRace = function (id) {
      $('#stopdiv').click();
      d3.select('svg').remove();
      if (typeof timer_ret_val != 'undefined') {
        timer_ret_val = true;
      }

      $http.get('/api/races/' + id).then(function (res) {
        $log.info('race ' + id + ' loaded with origin: [' + res.data.origin.x + ',' + res.data.origin.y + '].' );

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
        $scope.data = res.data.coords;
        $scope.x = x;
        $scope.y = y;
        $scope.raceIsLoaded = true;
      });
    };

    // Slider options with event handlers
    $scope.slider = {
      'options': {
        start: function (event, ui) { $scope.stopTimer(); }
      }
    };

    var ticks = 0;
    var last = 0;
    $scope.currentPos = 0;

    $scope.startTimer = function() {
      $scope.timer_ret_val = false;
      d3.timer(function(elapsed) {
        ticks = (ticks + (elapsed - last) / 20000) % 1;
        last = elapsed;
        $scope.currentPos = Math.round(ticks*$scope.data.length);
        $scope.$apply();
        return $scope.timer_ret_val;
      });
    };

    $scope.stopTimer = function() {
      $scope.timer_ret_val = true;
    };
  });