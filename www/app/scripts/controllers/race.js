'use strict';

angular.module('anemomindApp')
  .controller('RaceCtrl', function ($scope, Race, $http, $log) {
    $scope.races = Race.get();

    $scope.loadRace = function (id) {
      $('#stopdiv').click();
      d3.select('svg').remove();
      if (typeof timer_ret_val !== 'undefined') {
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
        $scope.coords = res.data.coords;
        $scope.data = res.data.data;
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
    $scope.toggleLabel = 'Pause';
    $scope.current = {};

    $scope.togglePlayPause = function(currentPos) {
      if ($scope.toggleLabel === 'Pause') {
        $scope.stopTimer();
      } else {
        $scope.startTimer(currentPos);
      }
    };

    $scope.startTimer = function(currentPos) {
      $scope.toggleLabel = 'Pause';
      $scope.timer_ret_val = false;
      if (arguments.length === 1) {
        last = 0;
        ticks = currentPos/$scope.coords.length;
      }
      d3.timer(function(elapsed) {
        ticks = (ticks + (elapsed - last) / 100000) % 1;
        last = elapsed;
        $scope.currentPos = Math.round(ticks*$scope.coords.length);
        $scope.$apply();
        return $scope.timer_ret_val;
      });
    };

    $scope.stopTimer = function() {
      $scope.toggleLabel = 'Play';
      $scope.timer_ret_val = true;
    };

    $scope.$watch('currentPos', function(currentPos) {
      if($scope.data) {
        var elapsedSeconds = ($scope.data[currentPos].timeMs - $scope.data[0].timeMs) / 1000;
        var hours = parseInt(elapsedSeconds / 3600) % 24;
        var minutes = parseInt(elapsedSeconds / 60) % 60;
        var seconds = parseInt(elapsedSeconds % 60, 10);
        $scope.current.elapsed = {
          hours: hours < 10 ? '0' + hours : hours,
          minutes: minutes < 10 ? '0' + minutes : minutes,
          seconds: seconds < 10 ? '0' + seconds : seconds
        };
        $scope.current.awaRad = $scope.data[currentPos].awaRad.toFixed(2);
        $scope.current.awsMps = $scope.data[currentPos].awsMps.toFixed(2);
        $scope.current.twaRad = $scope.data[currentPos].twaRad.toFixed(2);
        $scope.current.twsMps = $scope.data[currentPos].twsMps.toFixed(2);
        $scope.current.watSpeedMps = $scope.data[currentPos].watSpeedMps.toFixed(2);
        $scope.current.gpsSpeedMps = $scope.data[currentPos].gpsSpeedMps.toFixed(2);
        $scope.current.magHdgRad = $scope.data[currentPos].magHdgRad.toFixed(2);
        $scope.current.gpsBearingRad = $scope.data[currentPos].gpsBearingRad.toFixed(2);
      }
    });
  });